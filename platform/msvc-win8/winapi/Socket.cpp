// Socket.cpp

#include <Windows.h>

#undef WINAPI
#define WINAPI

#include "Socket.h"
#include "Iocp.h"

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
		, lpCompletionKey(0)
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
			stream_socket = ref new Windows::Networking::Sockets::StreamSocket();
		} else {
			datagram_socket = ref new Windows::Networking::Sockets::DatagramSocket();
			pointer_t shared_this(shared_from_this());
			datagram_socket->MessageReceived += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::DatagramSocket ^, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs ^>([shared_this](
				Windows::Networking::Sockets::DatagramSocket ^, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs ^ arg)
			{
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->read_datas_.push_back(arg->GetDataReader()->DetachBuffer());
				socket->read_data_size_ += socket->read_datas_.back()->Length;
				socket->handle_overlap_read();
				socket->cond_.notify_all();
			});
		}
	}

	int socket_t::bind(
		_In_  const struct sockaddr *name,
		_In_  int namelen)
	{
		if (type == SOCK_STREAM) {
			stream_socket = nullptr;
			stream_listener = ref new Windows::Networking::Sockets::StreamSocketListener();
			pointer_t shared_this(shared_from_this());
			stream_listener->ConnectionReceived += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::StreamSocketListener ^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs ^>([shared_this](
				Windows::Networking::Sockets::StreamSocketListener ^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs ^ arg)
			{
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->accept_sockets.push_back(arg->Socket);
				socket->handle_overlap_accept();
				socket->cond_.notify_all();
			});
			return wait_action(
				stream_listener->BindEndpointAsync(sockaddr_to_host_name(name), sockaddr_to_svc_name(name)));
		} else {
			return wait_action(
				datagram_socket->BindEndpointAsync(sockaddr_to_host_name(name), sockaddr_to_svc_name(name)));
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
		assert(stream_listener != nullptr);
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
			Windows::Foundation::IAsyncAction ^ action = type == SOCK_STREAM 
				? stream_socket->ConnectAsync(endp) 
				: datagram_socket->ConnectAsync(endp);
			pointer_t shared_this(shared_from_this());
			action->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler([shared_this](
				Windows::Foundation::IAsyncAction^ action, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->connecting_ = false;
				if (status == Windows::Foundation::AsyncStatus::Completed) {
					socket->status_ = s_establish;
				} else {
					socket->ec_ = SCODE_CODE(action->ErrorCode.Value);
				}
				socket->handle_overlap_connect();
				socket->cond_.notify_all();
			});
		}
		if (connecting_) {
			if (lpOverlapped) {
				overlap_task task(NULL, 0, lpOverlapped);
				write_tasks.push_back(task);
				le.set(WSA_IO_PENDING);
				return FALSE;
			} else if (flags_ & f_non_block) {
				le.set(WSAEINPROGRESS);
				return SOCKET_ERROR;
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
				iocp->push(ec_, lpOverlapped, 0);
			}
			le.set2(ec_);
			return SOCKET_ERROR;
		} else {
			if (lpOverlapped) {
				iocp->push(lpCompletionKey, lpOverlapped, 0);
			}
			return 0;
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

		if (!accept_sockets.empty()) {
			accept_conn(sock, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived);
			if (lpOverlapped) {
				iocp->push(lpCompletionKey, lpOverlapped, *lpdwBytesReceived);
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
			read_tasks.push_back(task);
			le.set(WSA_IO_PENDING);
			return FALSE;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return FALSE;
		} else {
			cond_.wait(lc, [this](){
				return ((status_ & s_can_read) == 0) || (!accept_sockets.empty());
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

		if (type == SOCK_STREAM) {
			tcp_recv_some();
		}

		if (read_data_size_ > 0) {
			read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
			if (lpOverlapped) {
				iocp->push(lpCompletionKey, lpOverlapped, *lpNumberOfBytesRecvd);
			}
			return 0;
		}

		if (lpOverlapped) {
			overlap_task task(lpBuffers, dwBufferCount, lpOverlapped);
			read_tasks.push_back(task);
			le.set(WSA_IO_PENDING);
			return SOCKET_ERROR;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		} else {
			cond_.wait(lc, [this](){
				return ec_ != 0 || read_data_size_ > 0;
			});
			if (ec_) {
				le.set2(ec_);
				return SOCKET_ERROR;
			} else {
				read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
				return 0;
			}
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
		return SOCKET_ERROR;
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

		if (type == SOCK_STREAM) {
			tcp_send_some();
		}

		if (read_data_size_ < read_data_capacity_) {
			write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent);
			if (lpOverlapped) {
				iocp->push(lpCompletionKey, lpOverlapped, *lpNumberOfBytesSent);
			}

			if (type == SOCK_STREAM) {
				tcp_send_some();
			}

			return 0;
		}

		if (lpOverlapped) {
			overlap_task task(lpBuffers, dwBufferCount, lpOverlapped);
			write_tasks.push_back(task);
			le.set(WSA_IO_PENDING);
			return SOCKET_ERROR;
		} else if (flags_ & f_non_block) {
			le.set(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		} else {
			cond_.wait(lc, [this](){
				return ec_ != 0 || read_data_size_ < read_data_capacity_;
			});
			if (ec_) {
				le.set2(ec_);
				return SOCKET_ERROR;
			} else {
				write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent);
				return 0;
			}
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
		return SOCKET_ERROR;
	}

	int socket_t::getsockname(
		_Out_    struct sockaddr *name,
		_Inout_  int *namelen)
	{
		Windows::Networking::HostName ^ host_name = type == SOCK_STREAM 
			? stream_socket->Information->LocalAddress 
			: datagram_socket->Information->LocalAddress;
		Platform::String ^ port = type == SOCK_STREAM 
			? stream_socket->Information->LocalPort 
			: datagram_socket->Information->LocalPort;
		return host_name_port_to_sockaddr(af, name, namelen, host_name, port);
	}

	int socket_t::getpeername(
		_Out_    struct sockaddr *name,
		_Inout_  int *namelen)
	{
		Windows::Networking::HostName ^ host_name = type == SOCK_STREAM 
			? stream_socket->Information->RemoteAddress 
			: datagram_socket->Information->RemoteAddress;
		Platform::String ^ port = type == SOCK_STREAM 
			? stream_socket->Information->RemotePort 
			: datagram_socket->Information->RemotePort;
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
		return SOCKET_ERROR;
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
			if (stream_listener) {
				delete stream_listener;
				stream_listener = nullptr;
			}
			if (stream_socket) {
				delete stream_socket;
				stream_socket = nullptr;
			}
		} else {
			if (datagram_socket) {
				delete datagram_socket;
				datagram_socket = nullptr;
			}
		}

		for (auto iter = read_tasks.begin(); iter != read_tasks.end(); ++iter) {
			overlap_task & task = *iter;
			iocp->push(WSA_OPERATION_ABORTED, task.lpOverlapped, 0);
		}
		for (auto iter = write_tasks.begin(); iter != write_tasks.end(); ++iter) {
			overlap_task & task = *iter;
			iocp->push(WSA_OPERATION_ABORTED, task.lpOverlapped, 0);
		}
		iocp.reset();

		cond_.notify_all();

		return 0;
	}

	void socket_t::attach_iocp(
		_In_  iocp_t * iocp, 
		_In_  ULONG_PTR CompletionKey)
	{
		this->iocp = boost::static_pointer_cast<iocp_t>(iocp->shared_from_this());
		lpCompletionKey = CompletionKey;
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
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		pointer_t shared_this(shared_from_this());
		action->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler([shared_this](
			Windows::Foundation::IAsyncAction^, Windows::Foundation::AsyncStatus) {
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->cond_.notify_all();
		});
		cond_.wait(lc, [=]() {
			return action->Status  != Windows::Foundation::AsyncStatus::Started;
		});
		Windows::Foundation::AsyncStatus status = action->Status;
		Windows::Foundation::HResult result = action->ErrorCode;
		WSASetLastError(SCODE_CODE(result.Value));
		return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
	}

	template <typename TResult, typename TProgress>
	int socket_t::wait_operation2(
		Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^ operation, 
		TResult & result)
	{
		std::unique_lock<std::recursive_mutex> lc(mutex_);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^, Windows::Foundation::AsyncStatus) {
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->cond_.notify_all();
		});
		cond_.wait(lc, [=]() {
			return operation->Status  != Windows::Foundation::AsyncStatus::Started;
		});
		Windows::Foundation::AsyncStatus status = operation->Status;
		WSASetLastError(SCODE_CODE(operation->ErrorCode.Value));
		result = operation->GetResults();
		return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
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
			stream_socket->InputStream->ReadAsync(buffer, size, Windows::Storage::Streams::InputStreamOptions::Partial);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<Windows::Storage::Streams::IBuffer ^, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer ^, UINT32>  ^ operation, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->reading_ = false;
				if (status == Windows::Foundation::AsyncStatus::Completed) {
					socket->read_datas_.push_back(operation->GetResults());
					if (socket->read_datas_.back()->Length == 0) {
						socket->status_ &= ~s_can_read;
						socket->status_ |= ~s_read_eof;
					} else {
						socket->read_data_size_ += socket->read_datas_.back()->Length;
						socket->tcp_recv_some();
					}
					socket->handle_overlap_read();
				} else {
					socket->ec_ = SCODE_CODE(operation->ErrorCode.Value);
				}
				socket->cond_.notify_all();
		});
	}

	void socket_t::tcp_send_some()
	{
		if (write_data_size_ == 0 || writing_ || !(status_ & s_can_write))
			return;
		Windows::Storage::Streams::IBuffer ^ buffer = write_datas_.front();
		writing_ = true;
		Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation = 
			stream_socket->OutputStream->WriteAsync(buffer);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<UINT32, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->writing_ = false;
				if (status == Windows::Foundation::AsyncStatus::Completed) {
					socket->write_data_size_ -= operation->GetResults();
					socket->write_datas_.pop_front();
					socket->handle_overlap_write();
					socket->tcp_send_some();
				} else {
					socket->ec_ = SCODE_CODE(operation->ErrorCode.Value);
				}
				socket->cond_.notify_all();
		});
	}

	void socket_t::handle_overlap_connect()
	{
		while (!write_tasks.empty()) {
			overlap_task & task = write_tasks.front();
			DWORD dwNumberOfBytesRecvd = 0;
			iocp->push(ec_ ? ec_ : lpCompletionKey, task.lpOverlapped, dwNumberOfBytesRecvd);
			write_tasks.pop_front();
		}
	}

	void socket_t::handle_overlap_accept()
	{
		while (!read_tasks.empty() && !accept_sockets.empty()) {
			overlap_task & task = read_tasks.front();
			DWORD dwNumberOfBytesRecvd = 0;
			accept_conn(
				(socket_t *)task.buffers[0].buf, 
				task.buffers[1].buf, 
				task.buffers[0].len, 
				task.buffers[1].len, 
				task.buffers[2].len, 
				&dwNumberOfBytesRecvd);
			iocp->push(lpCompletionKey, task.lpOverlapped, dwNumberOfBytesRecvd);
			read_tasks.pop_front();
		}
	}

	void socket_t::handle_overlap_read()
	{
		while (!read_tasks.empty() && read_data_size_ > 0) {
			overlap_task & task = read_tasks.front();
			DWORD dwNumberOfBytesRecvd = 0;
			read_data(task.buffers, task.dwBufferCount, &dwNumberOfBytesRecvd);
			iocp->push(lpCompletionKey, task.lpOverlapped, dwNumberOfBytesRecvd);
			read_tasks.pop_front();
		}
		if (status_ & s_read_eof) {
			while (!read_tasks.empty()) {
				overlap_task & task = read_tasks.front();
				iocp->push(lpCompletionKey, task.lpOverlapped, 0);
				read_tasks.pop_front();
			}
		}
	}

	void socket_t::handle_overlap_write()
	{
		while (!write_tasks.empty() && write_data_size_ < write_data_capacity_) {
			overlap_task & task = read_tasks.front();
			DWORD dwNumberOfBytesSent = 0;
			write_data(task.buffers, task.dwBufferCount, &dwNumberOfBytesSent);
			iocp->push(lpCompletionKey, task.lpOverlapped, dwNumberOfBytesSent);
			read_tasks.pop_front();
		}
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
		sock->stream_socket = accept_sockets.front();
		accept_sockets.pop_front();
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

	void socket_t::read_data(
		_Inout_  LPWSABUF lpBuffers,
		_In_     DWORD dwBufferCount,
		_Out_    LPDWORD lpNumberOfBytesRecvd)
	{
		DWORD i = 0;
		*lpNumberOfBytesRecvd = 0;
		do {
			Windows::Storage::Streams::IBuffer ^ buffer = read_datas_.front();
			Windows::Storage::Streams::DataReader ^ reader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
			if (buffer->Length < lpBuffers[i].len) {
				reader->ReadBytes(Platform::ArrayReference<unsigned char>((unsigned char *)lpBuffers[i].buf, buffer->Length));
				*lpNumberOfBytesRecvd += buffer->Length;
				lpBuffers[i].len -= buffer->Length;
				lpBuffers[i].buf += buffer->Length;
				read_datas_.pop_front();
				if (type == SOCK_DGRAM) {
					break;
				}
			} else if (buffer->Length > lpBuffers[i].len) {
				reader->ReadBytes(Platform::ArrayReference<unsigned char>((unsigned char *)lpBuffers[i].buf, lpBuffers[i].len));
				*lpNumberOfBytesRecvd += lpBuffers[i].len;
				++i;
			} else {
				reader->ReadBytes(Platform::ArrayReference<unsigned char>((unsigned char *)lpBuffers[i].buf, buffer->Length));
				*lpNumberOfBytesRecvd += buffer->Length;
				read_datas_.pop_front();
				if (type == SOCK_DGRAM) {
					break;
				}
				++i;
			}
			reader->DetachBuffer();
		} while (i < dwBufferCount && !read_datas_.empty());
		read_data_size_ -= *lpNumberOfBytesRecvd;
	}

	void socket_t::write_data(
		_In_   LPWSABUF lpBuffers,
		_In_   DWORD dwBufferCount,
		_Out_  LPDWORD lpNumberOfBytesSent)
	{
		ULONG left = (type == SOCK_STREAM) ? write_data_capacity_ - write_data_size_ : ULONG(-1);
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
	}

}
