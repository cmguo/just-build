// Socket.cpp

#include <Windows.h>

#undef WINAPI
#define WINAPI

#include "Socket.h"
#include "iocp.h"

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
				socket->read_datas_.push_back(arg->GetDataReader()->DetachBuffer());
				socket->read_data_size_ += socket->read_datas_.back()->Length;
				socket->udp_remotes_.push_back(std::make_pair(arg->RemoteAddress, arg->RemotePort));
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
			stream_socket_ = nullptr;
			stream_listener_ = ref new Windows::Networking::Sockets::StreamSocketListener();
			pointer_t shared_this(shared_from_this());
			stream_listener_->ConnectionReceived += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::StreamSocketListener ^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs ^>([shared_this](
				Windows::Networking::Sockets::StreamSocketListener ^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs ^ arg)
			{
				socket_t * socket = (socket_t *)shared_this.get();
				std::unique_lock<std::recursive_mutex> lc(socket->mutex_);
				socket->accept_sockets_.push_back(arg->Socket);
				socket->handle_overlap_accept();
				socket->cond_.notify_all();
			});
			return wait_action(
				stream_listener_->BindEndpointAsync(sockaddr_to_host_name(name), sockaddr_to_svc_name(name)));
		} else {
			status_ |= s_can_read;
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
			Windows::Foundation::IAsyncAction ^ action = type == SOCK_STREAM 
				? stream_socket_->ConnectAsync(endp) 
				: datagram_socket_->ConnectAsync(endp);
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

		if (type == SOCK_STREAM) {
			tcp_recv_some();
		}

		if (read_data_size_ > 0) {
			read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
			if (lpOverlapped) {
				iocp_->push(lpCompletionKey_, lpOverlapped, *lpNumberOfBytesRecvd);
			}
			return 0;
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
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		assert (type == SOCK_DGRAM);

		if (read_data_size_ > 0) {
			udp_recv_from(lpFrom, lpFromlen);
			read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
			if (lpOverlapped) {
				iocp_->push(lpCompletionKey_, lpOverlapped, *lpNumberOfBytesRecvd);
			}
			return 0;
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
			cond_.wait(lc, [this](){
				return ec_ != 0 || read_data_size_ > 0;
			});
			if (ec_) {
				le.set2(ec_);
				return SOCKET_ERROR;
			} else {
				udp_recv_from(lpFrom, lpFromlen);
				read_data(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
				return 0;
			}
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

		if (type == SOCK_STREAM) {
			tcp_send_some();
		}

		if (write_data_size_ < write_data_capacity_) {
			write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent);
			if (lpOverlapped) {
				iocp_->push(lpCompletionKey_, lpOverlapped, *lpNumberOfBytesSent);
			}

			if (type == SOCK_STREAM) {
				tcp_send_some();
			} else {
				udp_send();
			}

			return 0;
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
			cond_.wait(lc, [this](){
				return ec_ != 0 || write_data_size_ < write_data_capacity_;
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
		wsa_set_last_error le;
		std::unique_lock<std::recursive_mutex> lc(mutex_);

		assert (type == SOCK_DGRAM);

		write_data(lpBuffers, dwBufferCount, lpNumberOfBytesSent);
		udp_send_to(lpTo, iToLen);

		if (lpOverlapped) {
			iocp_->push(lpCompletionKey_, lpOverlapped, *lpNumberOfBytesSent);
		}

		return 0;
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
		for (auto iter = write_tasks_.begin(); iter != write_tasks_.end(); ++iter) {
			overlap_task & task = *iter;
			iocp_->push(WSA_OPERATION_ABORTED, task.lpOverlapped, 0);
		}
		iocp_.reset();

		cond_.notify_all();

		return 0;
	}

	void socket_t::attach_iocp(
		_In_  iocp_t * iocp, 
		_In_  ULONG_PTR CompletionKey)
	{
		this->iocp_ = boost::static_pointer_cast<iocp_t>(iocp->shared_from_this());
		lpCompletionKey_ = CompletionKey;
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
			stream_socket_->OutputStream->WriteAsync(buffer);
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

	void socket_t::udp_recv_from(
		_Out_    struct sockaddr *lpFrom,
		_Inout_  LPINT lpFromlen)
	{
		assert(!udp_remotes_.empty());

		host_name_port_to_sockaddr(af, lpFrom, lpFromlen, udp_remotes_.front().first, udp_remotes_.front().second);
	}

	void socket_t::udp_send()
	{
		assert(status_ & s_can_write);
		Windows::Storage::Streams::IBuffer ^ buffer = write_datas_.front();
		Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation = 
			datagram_socket_->OutputStream->WriteAsync(buffer);
		pointer_t shared_this(shared_from_this());
		operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<UINT32, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation, Windows::Foundation::AsyncStatus status) {
				socket_t * socket = (socket_t *)shared_this.get();
				if (status == Windows::Foundation::AsyncStatus::Error) {
					socket->ec_ = SCODE_CODE(operation->ErrorCode.Value);
				}
			});
	}

	void socket_t::udp_send_to(
			_In_   const struct sockaddr *lpTo,
			_In_   int iToLen)
	{
		assert(write_datas_.size() == 1);
		Windows::Storage::Streams::IBuffer ^ buffer = write_datas_.front();
		write_datas_.pop_front();
		write_data_size_ -= buffer->Length;
		Windows::Storage::Streams::IOutputStream ^ result;
		uint64_t key = ((uint64_t)((sockaddr_in *)lpTo)->sin_addr.s_addr << 32) | (((sockaddr_in *)lpTo)->sin_port);
		auto iter = udp_streams_.find(key);
		if (iter == udp_streams_.end()) {
			if (0 != wait_operation(
				datagram_socket_->GetOutputStreamAsync(sockaddr_to_host_name(lpTo), sockaddr_to_svc_name(lpTo)), result)) {
					ec_ = WSAGetLastError();
					return;
			}
			udp_streams_[key] = result;
		} else {
			result = iter->second;
		}
		pointer_t shared_this(shared_from_this());
		Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation2 = 
			result->WriteAsync(buffer);
		operation2->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<UINT32, UINT32>([shared_this](
			Windows::Foundation::IAsyncOperationWithProgress<UINT32, UINT32>  ^ operation, Windows::Foundation::AsyncStatus status){
				socket_t * socket = (socket_t *)shared_this.get();
				if (status == Windows::Foundation::AsyncStatus::Error) {
					socket->ec_ = SCODE_CODE(operation->ErrorCode.Value);
				}
		});
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
		while (!read_tasks_.empty() && read_data_size_ > 0) {
			overlap_task & task = read_tasks_.front();
			DWORD dwNumberOfBytesRecvd = 0;
			if (type == SOCK_DGRAM && task.lpFrom) {
				udp_recv_from(task.lpFrom, task.lpFromlen);
			}
			read_data(task.buffers, task.dwBufferCount, &dwNumberOfBytesRecvd);
			iocp_->push(lpCompletionKey_, task.lpOverlapped, dwNumberOfBytesRecvd);
			read_tasks_.pop_front();
		}
		if (status_ & s_read_eof) {
			while (!read_tasks_.empty()) {
				overlap_task & task = read_tasks_.front();
				iocp_->push(lpCompletionKey_, task.lpOverlapped, 0);
				read_tasks_.pop_front();
			}
		}
	}

	void socket_t::handle_overlap_write()
	{
		while (!write_tasks_.empty() && write_data_size_ < write_data_capacity_) {
			overlap_task & task = read_tasks_.front();
			DWORD dwNumberOfBytesSent = 0;
			write_data(task.buffers, task.dwBufferCount, &dwNumberOfBytesSent);
			iocp_->push(lpCompletionKey_, task.lpOverlapped, dwNumberOfBytesSent);
			read_tasks_.pop_front();
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
		sock->stream_socket_ = accept_sockets_.front();
		accept_sockets_.pop_front();
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
					udp_remotes_.pop_front();
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
