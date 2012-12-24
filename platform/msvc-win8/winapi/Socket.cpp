// Socket.cpp

#include <Windows.h>

#define WINAPI_DECL	 __declspec(dllexport)

#include "Socket.h"
#include "Iocp.h"
#include "Select.h"

namespace SocketEmulation
{

	class wsa_set_last_error
	{
	public:
		wsa_set_last_error()
			: ec_(0)
		{
		}
		
		void set(
			int ec)
		{
			ec_ = ec;
		}

		void set2(
			int & ec)
		{
			ec_ = ec;
			ec = 0;
		}

		int & get()
		{
			return ec_;
		}

		int ret() const
		{
			return ec_ == 0 ? 0 : SOCKET_ERROR;
		}

		~wsa_set_last_error()
		{
			WSASetLastError(ec_);
		}

	private:
		int ec_;
	};

	socket_t::socket_t()
		: af(AF_UNSPEC)
		, type(SOCK_RAW)
		, protocol(IPPROTO_IP)
		, lpCompletionKey_(0)
		, flags_(0)
		, status_(0)
		, read_data_size_(0)
		, write_data_size_(0)
		, read_data_capacity_(64 * 1024)
		, write_data_capacity_(64 * 1024)
		, connecting_(false)
		, reading_(false)
		, writing_(false)
		, ec_(0)
	{
		Windows::Foundation::Collections::IVectorView<Windows::Networking::HostName ^> ^ host_names = 
			Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
		for (unsigned int i = 0; i < host_names->Size; ++i) {
			OutputDebugString(host_names->GetAt(i)->RawName->Data());
		}
	}

	socket_t::~socket_t()
	{
	}

	void socket_t::create(
		_In_  int af,
		_In_  int type,
		_In_  int protocol)
	{
		this->af = af;
		this->type = type;
		this->protocol = protocol;
		if (type == SOCK_STREAM) {
			stream_socket_ = ref new Windows::Networking::Sockets::StreamSocket();
		} else {
			datagram_socket_ = ref new Windows::Networking::Sockets::DatagramSocket();
			pointer_t shared_this(shared_from_this());
			datagram_socket_->MessageReceived += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::DatagramSocket ^, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs ^>([shared_this](
				Windows::Networking::Sockets::DatagramSocket ^, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs ^ arg)
			{
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				try {
					Windows::Storage::Streams::IBuffer ^ buffer = arg->GetDataReader()->DetachBuffer();
					socket->read_datas_.push_back(buffer);
					socket->read_data_size_ += buffer->Length;
					socket->udp_read_addrs_.push_back(std::make_pair(arg->RemoteAddress, arg->RemotePort));
				} catch (Platform::Exception ^ e) {
					socket->ec_ = SCODE_CODE(e->HResult);
				} catch (...) {
					socket->ec_ = E_FAIL;
				}
				socket->handle_overlap_read();
				socket->handle_select_read();
				socket->cond_.notify_all();
			});
		}
	}

	int socket_t::bind(
		_In_  const struct sockaddr *name,
		_In_  int namelen)
	{
		if (type == SOCK_STREAM) {
			stream_socket_ = nullptr;
			stream_listener_ = ref new Windows::Networking::Sockets::StreamSocketListener();
			pointer_t shared_this(shared_from_this());
			stream_listener_->ConnectionReceived += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::StreamSocketListener ^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs ^>([shared_this](
				Windows::Networking::Sockets::StreamSocketListener ^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs ^ arg)
			{
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->accept_sockets_.push_back(arg->Socket);
				++socket->read_data_size_;
				socket->handle_overlap_accept();
				socket->handle_select_read();
				socket->cond_.notify_all();
			});
			return wait_action(
				stream_listener_->BindEndpointAsync(sockaddr_to_host_name(name), sockaddr_to_svc_name(name)));
		} else {
			status_ |= (s_can_read | s_can_write);
			return wait_action(
				datagram_socket_->BindEndpointAsync(sockaddr_to_host_name(name), sockaddr_to_svc_name(name)));
		}
	}

	int socket_t::connect(
		_In_  const struct sockaddr *name,
		_In_  int namelen)
	{
		return connect_ex(name, namelen, NULL) == TRUE ? 0 : SOCKET_ERROR;
	}

	int socket_t::listen(
		_In_  int backlog)
	{
		assert(stream_listener_ != nullptr);
		status_ = s_can_read;
		return 0;
	}

	BOOL socket_t::accept(
		_In_     socket_t * sock, 
		_Out_    struct sockaddr *addr, 
		_Inout_  int *addrlen)
	{
		char pOutputBuffer[512];
		DWORD dwBytesReceived = 0;
		BOOL ret = accept_ex(sock, pOutputBuffer, 0, 256, 256, &dwBytesReceived, NULL);
		if (ret == TRUE && addr != NULL && addrlen != NULL) {
			struct sockaddr *addr1 = NULL;
			struct sockaddr *addr2 = NULL;
			INT len1 = 0;
			INT len2 = 0;
			get_accept_ex_addrs(pOutputBuffer, 0, 256, 256, &addr1, &len1, &addr2, &len2);
			if (*addrlen >= len2) {
				memcpy(addrlen, addr2, len2);
			}
			*addrlen = len2;
		}
		return ret;
	}

	BOOL socket_t::connect_ex(
		_In_  const struct sockaddr *name,
		_In_  int namelen,
		_In_   LPOVERLAPPED lpOverlapped)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		if (!connecting_) {
			Windows::Networking::EndpointPair ^ endp = ref new Windows::Networking::EndpointPair(
				nullptr, nullptr, 
				sockaddr_to_host_name(name), 
				sockaddr_to_svc_name(name));
			connecting_ = true;
			write_data_size_ = write_data_capacity_;
			Windows::Foundation::IAsyncAction ^ action;
			if (type == SOCK_STREAM) {
				action = stream_socket_->ConnectAsync(endp);
			} else {
				action = datagram_socket_->ConnectAsync(endp);
			}
			pointer_t shared_this(shared_from_this());
			action->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler([shared_this](
				Windows::Foundation::IAsyncAction^ action, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				socket->on_connect(SCODE_CODE(action->ErrorCode.Value));
			});
		}
		if (lpOverlapped) {
			lpOverlapped->Internal = (ULONG_PTR)this;
		}
		if (connecting_) {
			if (lpOverlapped) {
				overlap_task task(NULL, 0, lpOverlapped);
				write_tasks_.push_back(task);
				le.set(WSA_IO_PENDING);
				return FALSE;
			} else if (flags_ & f_non_block) {
				le.set(WSAEINPROGRESS);
				return FALSE;
			} else {
				cond_.wait(lc, [this](){
					return ec_ != 0 || !connecting_;
				});
				if (ec_) {
					le.set2(ec_);
					return FALSE;
				} else {
					return TRUE;
				}
			}
		} else if (!(status_ & s_established)) {
			if (lpOverlapped) {
				iocp_->push(ec_, lpOverlapped, 0);
			}
			le.set2(ec_);
			return FALSE;
		} else {
			if (lpOverlapped) {
				iocp_->push(lpCompletionKey_, lpOverlapped, 0);
			}
			return TRUE;
		} 
	}

	BOOL socket_t::accept_ex(
		_In_   socket_t * sock,
		_In_   PVOID lpOutputBuffer,
		_In_   DWORD dwReceiveDataLength,
		_In_   DWORD dwLocalAddressLength,
		_In_   DWORD dwRemoteAddressLength,
		_Out_  LPDWORD lpdwBytesReceived,
		_In_   LPOVERLAPPED lpOverlapped)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		if (lpOverlapped) {
			lpOverlapped->Internal = (ULONG_PTR)this;
		}

		if (!accept_sockets_.empty()) {
			accept_conn(sock, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived);
			if (lpOverlapped) {
				iocp_->push(lpCompletionKey_, lpOverlapped, *lpdwBytesReceived);
			}
			return TRUE;
		}

		if (lpOverlapped) {
			WSABUF buf[] = {
				{dwReceiveDataLength, (char *)sock}, 
				{dwLocalAddressLength, (char *)lpOutputBuffer}, 
				{dwRemoteAddressLength, NULL}
			};
			overlap_task task(buf, 3, lpOverlapped);
			read_tasks_.push_back(task);
			le.set(WSA_IO_PENDING);
			return FALSE;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return FALSE;
		} else {
			cond_.wait(lc, [this](){
				return ((status_ & s_can_read) == 0) || (!accept_sockets_.empty());
			});
			if ((status_ & s_can_read) == 0) {
				le.set(WSAESHUTDOWN);
				return FALSE;
			} else {
				accept_conn(sock, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived);
				return TRUE;
			}
		}
	}

	void socket_t::get_accept_ex_addrs(
		_In_   PVOID lpOutputBuffer,
		_In_   DWORD dwReceiveDataLength,
		_In_   DWORD dwLocalAddressLength,
		_In_   DWORD dwRemoteAddressLength,
		_Out_  LPSOCKADDR *LocalSockaddr,
		_Out_  LPINT LocalSockaddrLength,
		_Out_  LPSOCKADDR *RemoteSockaddr,
		_Out_  LPINT RemoteSockaddrLength)
	{
		int * namelen = (int *)((char *)lpOutputBuffer + dwReceiveDataLength);
		*LocalSockaddr = (sockaddr *)(namelen + 1);
		*LocalSockaddrLength = *namelen;
		namelen = (int *)((char *)lpOutputBuffer + dwReceiveDataLength + dwLocalAddressLength);
		*RemoteSockaddr = (sockaddr *)(namelen + 1);
		*RemoteSockaddrLength = *namelen;
	}

	int socket_t::recv_ex(
		_Inout_  LPWSABUF lpBuffers,
		_In_     DWORD dwBufferCount,
		_Out_    LPDWORD lpNumberOfBytesRecvd,
		_Inout_  LPDWORD lpFlags,
		_In_     LPWSAOVERLAPPED lpOverlapped,
		_In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
		)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		if (lpOverlapped) {
			lpOverlapped->Internal = (ULONG_PTR)this;
		}

		if (type == SOCK_STREAM) {
			tcp_recv_some();
		}

		INT * pErrorCode = &le.get();
		if (read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, NULL, NULL, lpOverlapped, pErrorCode)) {
			return le.ret();
		}

		if (lpOverlapped) {
			overlap_task task(lpBuffers, dwBufferCount, lpOverlapped);
			read_tasks_.push_back(task);
			le.set(WSA_IO_PENDING);
			return SOCKET_ERROR;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		} else {
			cond_.wait(lc, [this, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, pErrorCode](){
				return read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, NULL, NULL, NULL, pErrorCode);
			});
			return le.ret();
		}
	}

	int socket_t::recv_from_ex(
		_Inout_  LPWSABUF lpBuffers,
		_In_     DWORD dwBufferCount,
		_Out_    LPDWORD lpNumberOfBytesRecvd,
		_Inout_  LPDWORD lpFlags,
		_Out_    struct sockaddr *lpFrom,
		_Inout_  LPINT lpFromlen,
		_In_     LPWSAOVERLAPPED lpOverlapped,
		_In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		assert (type == SOCK_DGRAM);

		if (lpOverlapped) {
			lpOverlapped->Internal = (ULONG_PTR)this;
		}

		INT * pErrorCode = &le.get();
		if (read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFrom, lpFromlen, lpOverlapped, pErrorCode)) {
			return le.ret();
		}

		if (lpOverlapped) {
			overlap_task task(lpBuffers, dwBufferCount, lpFrom, lpFromlen, lpOverlapped);
			read_tasks_.push_back(task);
			le.set(WSA_IO_PENDING);
			return SOCKET_ERROR;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		} else {
			cond_.wait(lc, [this, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFrom, lpFromlen, pErrorCode](){
				return read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFrom, lpFromlen, NULL, pErrorCode);
			});
			return le.ret();
		}
	}

	int socket_t::send_ex(
		_In_   LPWSABUF lpBuffers,
		_In_   DWORD dwBufferCount,
		_Out_  LPDWORD lpNumberOfBytesSent,
		_In_   DWORD dwFlags,
		_In_   LPWSAOVERLAPPED lpOverlapped,
		_In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		if (lpOverlapped) {
			lpOverlapped->Internal = (ULONG_PTR)this;
		}

		INT * pErrorCode = &le.get();
		if (write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent, NULL, NULL, lpOverlapped, pErrorCode)) {
			if (type == SOCK_STREAM) {
				tcp_send_some();
			} else {
				udp_send();
			}
			return le.ret();
		}

		if (lpOverlapped) {
			overlap_task task(lpBuffers, dwBufferCount, lpOverlapped);
			write_tasks_.push_back(task);
			le.set(WSA_IO_PENDING);
			return SOCKET_ERROR;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		} else {
			cond_.wait(lc, [this, lpBuffers, dwBufferCount, lpNumberOfBytesSent, pErrorCode](){
				return write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent, NULL, NULL, NULL, pErrorCode);
			});
			if (type == SOCK_STREAM) {
				tcp_send_some();
			} else {
				udp_send();
			}
			return le.ret();
		}
	}

	int socket_t::send_to_ex(
		_In_   LPWSABUF lpBuffers,
		_In_   DWORD dwBufferCount,
		_Out_  LPDWORD lpNumberOfBytesSent,
		_In_   DWORD dwFlags,
		_In_   const struct sockaddr *lpTo,
		_In_   int iToLen,
		_In_   LPWSAOVERLAPPED lpOverlapped,
		_In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
		)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		assert (type == SOCK_DGRAM);

		if (lpOverlapped) {
			lpOverlapped->Internal = (ULONG_PTR)this;
		}

		INT * pErrorCode = &le.get();
		if (write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent, lpTo, iToLen, lpOverlapped, pErrorCode)) {
			udp_send();
			return le.ret();
		}

		if (lpOverlapped) {
			overlap_task task(lpBuffers, dwBufferCount, lpOverlapped);
			write_tasks_.push_back(task);
			le.set(WSA_IO_PENDING);
			return SOCKET_ERROR;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		} else {
			cond_.wait(lc, [this, lpBuffers, dwBufferCount, lpNumberOfBytesSent, pErrorCode](){
				return write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent, NULL, NULL, NULL, pErrorCode);
			});
			udp_send();
			return le.ret();
		}
	}

	int socket_t::getsockname(
		_Out_    struct sockaddr *name,
		_Inout_  int *namelen)
	{
		Windows::Networking::HostName ^ host_name = type == SOCK_STREAM 
			? stream_socket_->Information->LocalAddress 
			: datagram_socket_->Information->LocalAddress;
		Platform::String ^ port = type == SOCK_STREAM 
			? stream_socket_->Information->LocalPort 
			: datagram_socket_->Information->LocalPort;
		return host_name_port_to_sockaddr(af, name, namelen, host_name, port);
	}

	int socket_t::getpeername(
		_Out_    struct sockaddr *name,
		_Inout_  int *namelen)
	{
		Windows::Networking::HostName ^ host_name = type == SOCK_STREAM 
			? stream_socket_->Information->RemoteAddress 
			: datagram_socket_->Information->RemoteAddress;
		Platform::String ^ port = type == SOCK_STREAM 
			? stream_socket_->Information->RemotePort 
			: datagram_socket_->Information->RemotePort;
		return host_name_port_to_sockaddr(af, name, namelen, host_name, port);
	}

	int socket_t::shutdown(
		_In_  int how)
	{
		return SOCKET_ERROR;
	}

	int socket_t::setsockopt(
		_In_  int level,
		_In_  int optname,
		_In_  const char *optval,
		_In_  int optlen)
	{
		return SOCKET_ERROR;
	}

	int socket_t::getsockopt(
		_In_     int level,
		_In_     int optname,
		_Out_    char *optval,
		_Inout_  int *optlen)
	{
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		switch (level) {
		case SOL_SOCKET:
			switch (optname)
			{
			case SO_ERROR:
				*(int *)optval = ec_;
				ec_ = 0;
				break;
			case SO_CONNECT_TIME:
				break;
			default:
				le.set(WSAEINVAL);
				break;
			}
			break;
		default:
			le.set(WSAEINVAL);
			break;
		};
		return le.ret();
	}

	int socket_t::ioctlsocket(
		_In_     long cmd,
		_Inout_  u_long *argp)
	{
		switch (cmd) {
		case FIONBIO:
			if (*argp == 1) {
				flags_ |= f_non_block;
			} else {
				flags_ &= ~f_non_block;
			}
			return 0;
		default:
			return SOCKET_ERROR;
		};
	}

	int socket_t::close()
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		status_ = 0;

		if (type == SOCK_STREAM) {
			if (stream_listener_) {
				delete stream_listener_;
				stream_listener_ = nullptr;
				accept_sockets_.clear();
			}
			if (stream_socket_) {
				delete stream_socket_;
				stream_socket_ = nullptr;
			}
		} else {
			if (datagram_socket_) {
				delete datagram_socket_;
				datagram_socket_ = nullptr;
			}
			udp_streams_.clear();
		}

		for (auto iter = read_tasks_.begin(); iter != read_tasks_.end(); ++iter) {
			overlap_task & task = *iter;
			iocp_->push(WSA_OPERATION_ABORTED, task.lpOverlapped, 0);
		}
		read_tasks_.clear();
		for (auto iter = write_tasks_.begin(); iter != write_tasks_.end(); ++iter) {
			overlap_task & task = *iter;
			iocp_->push(WSA_OPERATION_ABORTED, task.lpOverlapped, 0);
		}
		write_tasks_.clear();
		iocp_.reset();

		cond_.notify_all();

		return 0;
	}

	void socket_t::attach_iocp(
		_In_  iocp_t * iocp, 
		_In_  ULONG_PTR CompletionKey)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		this->iocp_ = boost::static_pointer_cast<iocp_t>(iocp->shared_from_this());
		lpCompletionKey_ = CompletionKey;
	}

	void socket_t::select_attach(
		_In_  int t,
		_In_  select_t * select)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		switch (t) 
		{
		case 0: // read
			if (read_data_size_ > 0) {
				select->set(t, index);
				select = NULL;
			}
			break;
		case 1: // write
			if (write_data_size_ < write_data_capacity_) {
				select->set(t, index);
				select = NULL;
			}
			break;
		case 2: // except
			if (ec_) {
				select->set(t, index);
				select = NULL;
			}
			break;
		default:
			break;
		}
		if (select) {
			select_tasks_[t].push_back(select);
		}
	}

	void socket_t::select_detach(
		_In_  int t,
		_In_  select_t * select)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		auto iter = std::find(select_tasks_[t].begin(), select_tasks_[t].end(), select);
		if (iter != select_tasks_[t].end()) {
			select_tasks_[t].erase(iter);
		}
	}

	Windows::Networking::HostName ^ socket_t::sockaddr_to_host_name(
		_In_  const struct sockaddr *name)
	{
		WCHAR wstr[256];
		DWORD len = 256;
		if (name->sa_family == AF_INET) {
			if (((sockaddr_in *)name)->sin_addr.s_addr == 0) {
				// bind on 0.0.0.0 will fail, but a null HostName will take effect!!
				return nullptr;
			}
			WSAAddressToStringW((sockaddr *)name, sizeof(sockaddr_in), NULL, wstr, &len);
		} else {
			//WSAAddressToStringW(((LPSOCKADDR)name, sizeof(sockaddr_in6), NULL, wstr, &len);
		}
		return ref new Windows::Networking::HostName(ref new Platform::String(wstr));
	}

	Platform::String ^ socket_t::sockaddr_to_svc_name(
		_In_  const struct sockaddr *name)
	{
		WCHAR wstr[8];
		u_short port = 0;
		if (name->sa_family == AF_INET) {
			port = ((sockaddr_in *)name)->sin_port;
		} else {
			//WSAAddressToStringW(((LPSOCKADDR)name, sizeof(sockaddr_in6), NULL, wstr, &len);
		}
		port = ntohs(port);
		WCHAR * p = wstr + 7;
		*p = L'\0';
		while (port) {
			--p;
			*p = port % 10 + L'0';
			port = port / 10;
		}
		if (*p == L'\0') {
			--p;
			*p = L'0';
		}
		return ref new Platform::String(p);
	}

	int socket_t::host_name_port_to_sockaddr(
		_In_  int af,
		_Out_    struct sockaddr *name,
		_Inout_  int *namelen, 
		Windows::Networking::HostName ^ host_name, 
		Platform::String ^ svc)
	{
		INT ret = WSAStringToAddressW((LPWSTR)host_name->RawName->Data(), af, NULL, name, namelen);
		char16 const * p = svc->Data();
		u_short port = 0;
		while (*p) {
			port = port * 10 + (*p - L'0');
			++p;
		}
		port = htons(port);
		if (af == AF_INET) {
			((sockaddr_in *)name)->sin_port = port;
		} else {
		}
		return ret;
	}

	DWORD socket_t::read_buffer(
		Windows::Storage::Streams::IBuffer ^ buffer, 
		_Inout_  LPWSABUF lpBuffers,
		_In_     DWORD dwBufferCount)
	{
		Windows::Storage::Streams::DataReader ^ reader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
		for (DWORD i = 0; i < dwBufferCount; ++i) {
			Platform::Array<uint8> ^ array = ref new Platform::Array<uint8>(lpBuffers[i].len);
			reader->ReadBytes(array);
			memcpy(lpBuffers[i].buf, array->Data, array->Length);
		}
		return buffer->Length;
	}

	DWORD socket_t::write_buffer(
		Windows::Storage::Streams::IBuffer ^ buffer, 
		_Inout_  LPWSABUF lpBuffers,
		_In_     DWORD dwBufferCount)
	{
		Windows::Storage::Streams::DataWriter ^ writer = ref new Windows::Storage::Streams::DataWriter();
		for (DWORD i = 0; i < dwBufferCount; ++i) {
			writer->WriteBytes(ref new Platform::Array<uint8>((uint8 *)lpBuffers[i].buf, lpBuffers[i].len));
		}
		buffer = writer->DetachBuffer();
		return buffer->Length;
	}

	int socket_t::wait_action(
		Windows::Foundation::IAsyncAction ^ action)
	{
		HANDLE hEvent = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		action->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler([hEvent](
			Windows::Foundation::IAsyncAction^, Windows::Foundation::AsyncStatus) {
				SetEvent(hEvent);
		});
		WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
		CloseHandle(hEvent);
		Windows::Foundation::AsyncStatus status = action->Status;
		WSASetLastError(SCODE_CODE(action->ErrorCode.Value));
		return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
	}

	template <typename TResult>
	int socket_t::wait_operation(
		Windows::Foundation::IAsyncOperation<TResult>  ^ operation, 
		TResult & result)
	{
		HANDLE hEvent = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		operation->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<TResult>([hEvent](
			Windows::Foundation::IAsyncOperation<TResult>  ^, Windows::Foundation::AsyncStatus) {
				SetEvent(hEvent);
		});
		WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
		CloseHandle(hEvent);
		Windows::Foundation::AsyncStatus status = operation->Status;
		result = operation->GetResults();
		WSASetLastError(SCODE_CODE(operation->ErrorCode.Value));
		return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
	}

	template <typename TResult, typename TProgress>
	int socket_t::wait_operation2(
		Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^ operation, 
		TResult & result)
	{
		HANDLE hEvent = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>([hEvent](
			Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^, Windows::Foundation::AsyncStatus) {
				SetEvent(hEvent);
		});
		WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
		CloseHandle(hEvent);
		Windows::Foundation::AsyncStatus status = operation->Status;
		result = operation->GetResults();
		WSASetLastError(SCODE_CODE(operation->ErrorCode.Value));
		return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
	}

	void socket_t::on_connect(
		int ec)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		connecting_ = false;
		write_data_size_ = 0;
		if (ec == 0) {
			status_ = s_establish;
			handle_select_write();
		} else {
			ec_ = ec;
			handle_select_except();
		}
		handle_overlap_connect();
		cond_.notify_all();
	}

	void socket_t::tcp_recv_some()
	{
		if (read_data_size_ >= read_data_capacity_ || reading_ || !(status_ & s_can_read))
			return;
		UINT32 size = 4096;
		if (size + read_data_size_ > read_data_capacity_) {
			size = read_data_capacity_ - read_data_size_;
		}
		Windows::Storage::Streams::IBuffer ^ buffer = ref new Windows::Storage::Streams::Buffer(size);
		reading_ = true;
		Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer ^, UINT32>  ^ operation = 
			stream_socket_->InputStream->ReadAsync(buffer, size, Windows::Storage::Streams::InputStreamOptions::Partial);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<Windows::Storage::Streams::IBuffer ^, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer ^, UINT32> ^ operation, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				if (status == Windows::Foundation::AsyncStatus::Completed) {
					socket->read_datas_.push_back(operation->GetResults());
					socket->tcp_on_recv(0, operation->GetResults()->Length);
				} else {
					socket->tcp_on_recv(SCODE_CODE(operation->ErrorCode.Value), 0);
				}
		});
	}

	void socket_t::tcp_send_some()
	{
		if (write_data_size_ == 0 || writing_ || !(status_ & s_can_write))
			return;
		Windows::Storage::Streams::IBuffer ^ buffer = write_datas_.front();
		writing_ = true;
		Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation = 
			stream_socket_->OutputStream->WriteAsync(buffer);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<UINT32, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32> ^ operation, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				if (status == Windows::Foundation::AsyncStatus::Completed) {
					socket->write_datas_.pop_front();
					socket->tcp_on_send(0, operation->GetResults());
				} else {
					socket->tcp_on_send(SCODE_CODE(operation->ErrorCode.Value), 0);
				}
		});
	}

	void socket_t::tcp_on_recv(
		int ec, 
		size_t size)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		reading_ = false;
		if (ec == 0) {
			if (size == 0) {
				status_ &= ~s_can_read;
				status_ |= s_read_eof;
			} else {
				read_data_size_ += size;
			}
			handle_overlap_read();
			handle_select_read();
			tcp_recv_some();
		} else {
			status_ &= ~s_can_read;
			ec_ = ec;
			handle_select_except();
		}
		cond_.notify_all();
	}

	void socket_t::tcp_on_send(
		int ec, 
		size_t size)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		writing_ = false;
		if (ec == 0) {
			write_data_size_ -= size;
			handle_overlap_write();
			handle_select_write();
			tcp_send_some();
		} else {
			status_ &= ~s_can_write;
			ec_ = ec;
			handle_select_except();
		}
		cond_.notify_all();
	}

	void socket_t::udp_send()
	{
		if (write_data_size_ == 0 || !(status_ & s_can_write))
			return;
		assert(write_datas_.size() == 1);
		Windows::Storage::Streams::IBuffer ^ buffer = write_datas_.front();
		write_datas_.pop_front();
		write_data_size_ -= buffer->Length;
		Windows::Storage::Streams::IOutputStream ^ stream = udp_write_addrs_.front();
		udp_write_addrs_.pop_front();
		Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation = stream->WriteAsync(buffer);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<UINT32, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				if (status == Windows::Foundation::AsyncStatus::Completed) {
					socket->udp_on_send(0, operation->GetResults());
				} else {
					socket->udp_on_send(SCODE_CODE(operation->ErrorCode.Value), 0);
				}
			});
	}

	void socket_t::udp_on_recv(
		int ec, 
		size_t size)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		read_data_size_ += size;
		handle_overlap_read();
		handle_select_read();
		cond_.notify_all();
	}

	void socket_t::udp_on_send(
		int ec, 
		size_t size)
	{
		if (ec) {
			std::unique_lock<std::recursive_mutex> lc(mutex_);
			ec_ = ec;
		}
	}

	void socket_t::handle_overlap_connect()
	{
		while (!write_tasks_.empty()) {
			overlap_task & task = write_tasks_.front();
			DWORD dwNumberOfBytesRecvd = 0;
			iocp_->push(ec_ ? ec_ : lpCompletionKey_, task.lpOverlapped, dwNumberOfBytesRecvd);
			write_tasks_.pop_front();
		}
	}

	void socket_t::handle_overlap_accept()
	{
		while (!read_tasks_.empty() && !accept_sockets_.empty()) {
			overlap_task & task = read_tasks_.front();
			DWORD dwNumberOfBytesRecvd = 0;
			accept_conn(
				(socket_t *)task.buffers[0].buf, 
				task.buffers[1].buf, 
				task.buffers[0].len, 
				task.buffers[1].len, 
				task.buffers[2].len, 
				&dwNumberOfBytesRecvd);
			iocp_->push(lpCompletionKey_, task.lpOverlapped, dwNumberOfBytesRecvd);
			read_tasks_.pop_front();
		}
	}

	void socket_t::handle_overlap_read()
	{
		while (!read_tasks_.empty()) {
			overlap_task & task = read_tasks_.front();
			DWORD dwNumberOfBytesRecvd = 0;
			if (read_data(task.buffers, task.dwBufferCount, &dwNumberOfBytesRecvd, task.lpFrom, task.lpFromlen, task.lpOverlapped, NULL)) {
				read_tasks_.pop_front();
			} else {
				break;
			}
		}
	}

	void socket_t::handle_overlap_write()
	{
		while (!write_tasks_.empty()) {
			overlap_task & task = write_tasks_.front();
			DWORD dwNumberOfBytesSent = 0;
			if (write_data(task.buffers, task.dwBufferCount, &dwNumberOfBytesSent, NULL, 0, task.lpOverlapped, NULL)) {
				write_tasks_.pop_front();
			} else {
				break;
			}
		}
	}

	void socket_t::handle_select_read()
	{
		if (read_data_size_ > 0 || (status_ & s_read_eof)) {
			handle_select(0);
		}
	}

	void socket_t::handle_select_write()
	{
		if (write_data_size_ < write_data_capacity_ || (status_ & s_read_eof)) {
			handle_select(1);
		}
	}

	void socket_t::handle_select_except()
	{
		if (ec_) {
			handle_select(2);
		}
	}

	void socket_t::handle_select(
		_In_  int t)
	{
		for (size_t i = 0; i < select_tasks_[t].size(); ++i) {
			select_tasks_[t][i]->set(t, index);
		}
		select_tasks_[t].clear();
	}

	void socket_t::accept_conn(
		_In_   socket_t * sock,
		_In_   PVOID lpOutputBuffer,
		_In_   DWORD dwReceiveDataLength,
		_In_   DWORD dwLocalAddressLength,
		_In_   DWORD dwRemoteAddressLength,
		_Out_  LPDWORD lpdwBytesReceived)
	{
		sock->af = af;
		sock->type = type;
		sock->protocol = protocol;
		sock->stream_socket_ = accept_sockets_.front();
		accept_sockets_.pop_front();
		--read_data_size_;
		sock->status_ = s_established | s_can_read | s_can_write;

		*lpdwBytesReceived = 0;

		if (lpOutputBuffer) {
			int * namelen = (int *)((char *)lpOutputBuffer + dwReceiveDataLength);
			*namelen = dwLocalAddressLength;
			sock->getsockname((sockaddr *)(namelen + 1), namelen);
			namelen = (int *)((char *)lpOutputBuffer + dwReceiveDataLength + dwLocalAddressLength);
			*namelen = dwRemoteAddressLength;
			sock->getpeername((sockaddr *)(namelen + 1), namelen);
		}
	}

	bool socket_t::read_data(
		_Inout_  LPWSABUF lpBuffers,
		_In_     DWORD dwBufferCount,
		_Out_    LPDWORD lpNumberOfBytesRecvd, 
		_Out_    struct sockaddr *lpFrom,
		_Inout_  LPINT lpFromlen,
		_In_     LPWSAOVERLAPPED lpOverlapped,
		_Out_    LPINT ErrorCode)
	{
		if (read_data_size_ == 0) {
			if (status_ & s_read_eof) {
				status_ &= ~s_read_eof;
				*lpNumberOfBytesRecvd = 0;
				if (lpOverlapped) {
					iocp_->push(lpCompletionKey_, lpOverlapped, 0);
				}
				if (ErrorCode)
					*ErrorCode = 0;
				return true;
			} else if (ec_) {
				if (ErrorCode) {
					*ErrorCode = ec_;
				} else if (lpOverlapped) {
					iocp_->push(ec_, lpOverlapped, 0);
				}
				ec_ = 0;
				return true;
			} else if (!(status_ & s_can_read)) {
				if (ErrorCode) {
					*ErrorCode = WSAECONNABORTED;
				} else if (lpOverlapped) {
					iocp_->push(WSAECONNABORTED, lpOverlapped, 0);
				}
				return true;
			} else {
				return false;
			}
		}

		DWORD i = 0;
		*lpNumberOfBytesRecvd = 0;
		do {
			Windows::Storage::Streams::IBuffer ^ buffer = read_datas_.front();
			Windows::Storage::Streams::DataReader ^ reader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
			UINT32 buffer_len = buffer->Length;
			if (buffer_len < lpBuffers[i].len) {
				reader->ReadBytes(Platform::ArrayReference<unsigned char>((unsigned char *)lpBuffers[i].buf, buffer_len));
				*lpNumberOfBytesRecvd += buffer_len;
				lpBuffers[i].len -= buffer_len;
				lpBuffers[i].buf += buffer_len;
				read_data_size_ -= buffer_len;
				read_datas_.pop_front();
				buffer_len = buffer->Length;
				if (type == SOCK_DGRAM) {
					if (lpFrom) {
						host_name_port_to_sockaddr(af, lpFrom, lpFromlen, udp_read_addrs_.front().first, udp_read_addrs_.front().second);
					}
					udp_read_addrs_.pop_front();
					break;
				}
			} else if (buffer_len > lpBuffers[i].len) {
				reader->ReadBytes(Platform::ArrayReference<unsigned char>((unsigned char *)lpBuffers[i].buf, lpBuffers[i].len));
				*lpNumberOfBytesRecvd += lpBuffers[i].len;
				read_data_size_ -= lpBuffers[i].len;
				++i;
			} else {
				reader->ReadBytes(Platform::ArrayReference<unsigned char>((unsigned char *)lpBuffers[i].buf, buffer_len));
				*lpNumberOfBytesRecvd += buffer_len;
				read_data_size_ -= buffer_len;
				read_datas_.pop_front();
				if (type == SOCK_DGRAM) {
					break;
				}
				++i;
			}
			reader->DetachBuffer();
		} while (i < dwBufferCount && read_data_size_ > 0);

		if (lpOverlapped) {
			iocp_->push(lpCompletionKey_, lpOverlapped, *lpNumberOfBytesRecvd);
		}
		if (ErrorCode)
			*ErrorCode = 0;

		return true;
	}

	bool socket_t::write_data(
		_In_   LPWSABUF lpBuffers,
		_In_   DWORD dwBufferCount,
		_Out_  LPDWORD lpNumberOfBytesSent,
		_In_   const struct sockaddr *lpTo,
		_In_   int iToLen,
		_In_   LPWSAOVERLAPPED lpOverlapped,
		_Out_  LPINT ErrorCode)
	{
		if (ec_) {
			if (ErrorCode) {
				*ErrorCode = ec_;
			} else if (lpOverlapped) {
				iocp_->push(ec_, lpOverlapped, 0);
			}
			ec_ = 0;
			return true;
		} else if (!(status_ & s_can_write)) {
			if (ErrorCode) {
				*ErrorCode = WSAECONNABORTED;
			} else if (lpOverlapped) {
				iocp_->push(WSAECONNABORTED, lpOverlapped, 0);
			}
			return true;
		} else if (write_data_capacity_ <= write_data_size_) {
			return false;
		}

		ULONG left = write_data_capacity_ - write_data_size_;

		if (type == SOCK_DGRAM) {
			left = ULONG(-1);
			Windows::Storage::Streams::IOutputStream ^ stream;
			if (lpTo) {
				uint64_t key = ((uint64_t)((sockaddr_in *)lpTo)->sin_addr.s_addr << 32) | (((sockaddr_in *)lpTo)->sin_port);
				auto iter = udp_streams_.find(key);
				if (iter == udp_streams_.end()) {
					if (0 != wait_operation(
						datagram_socket_->GetOutputStreamAsync(sockaddr_to_host_name(lpTo), sockaddr_to_svc_name(lpTo)), stream)) {
							int ec = WSAGetLastError();
							if (ErrorCode) {
								*ErrorCode = ec;
							} else if (lpOverlapped) {
								iocp_->push(ec, lpOverlapped, *lpNumberOfBytesSent);
							}
							return true;
					}
					udp_streams_[key] = stream;
				} else {
					stream = iter->second;
				}
			} else {
				stream = datagram_socket_->OutputStream;
			}
			udp_write_addrs_.push_back(stream);
		}

		Windows::Storage::Streams::DataWriter ^ writer = ref new Windows::Storage::Streams::DataWriter();
		for (DWORD i = 0; i < dwBufferCount; ++i) {
			if (lpBuffers[i].len < left) {
				writer->WriteBytes(Platform::ArrayReference<uint8>((uint8 *)lpBuffers[i].buf, lpBuffers[i].len));
				*lpNumberOfBytesSent += lpBuffers[i].len;
				left -= lpBuffers[i].len;
			} else {
				writer->WriteBytes(Platform::ArrayReference<uint8>((uint8 *)lpBuffers[i].buf, left));
				*lpNumberOfBytesSent += left;
				break;
			}
		}
		Windows::Storage::Streams::IBuffer ^ buffer = writer->DetachBuffer();
		write_datas_.push_back(buffer);
		write_data_size_ += *lpNumberOfBytesSent;

		if (lpOverlapped) {
			iocp_->push(lpCompletionKey_, lpOverlapped, *lpNumberOfBytesSent);
		}
		if (ErrorCode)
			*ErrorCode = 0;

		return true;
	}

}
