// Socket.h

#pragma once

#include "WsaContext.h"

#include <deque>
#include <mutex>
#include <condition_variable>

#include <assert.h>

namespace SocketEmulation
{

	struct overlap_task
	{
		overlap_task(
			_In_   LPWSABUF lpBuffers,
			_In_   DWORD dwBufferCount,
			_In_   LPWSAOVERLAPPED lpOverlapped)
			: dwBufferCount(dwBufferCount)
			, lpOverlapped(lpOverlapped)
		{
			std::copy(lpBuffers, lpBuffers + dwBufferCount, buffers);
		}

		WSABUF buffers[16];
		DWORD dwBufferCount;
		LPWSAOVERLAPPED lpOverlapped;
	};

	struct iocp_t;

	struct socket_t
		: wsa_handle_t<socket_t, 1>
	{
		socket_t();

		~socket_t();

		void create(
			_In_  int af,
			_In_  int type,
			_In_  int protocol);

		int bind(
			_In_  const struct sockaddr *name,
			_In_  int namelen);

		int connect(
			_In_  const struct sockaddr *name,
			_In_  int namelen);

		int listen(
		_In_  int backlog);

		int accept(
			_In_     socket_t * sock,
			_Out_    struct sockaddr *addr,
			_Inout_  int *addrlen);

		BOOL connect_ex(
			_In_  const struct sockaddr *name,
			_In_  int namelen,
			_In_   LPOVERLAPPED lpOverlapped);

		BOOL accept_ex(
			_In_   socket_t * sock,
			_In_   PVOID lpOutputBuffer,
			_In_   DWORD dwReceiveDataLength,
			_In_   DWORD dwLocalAddressLength,
			_In_   DWORD dwRemoteAddressLength,
			_Out_  LPDWORD lpdwBytesReceived,
			_In_   LPOVERLAPPED lpOverlapped);

		static void get_accept_ex_addrs(
			_In_   PVOID lpOutputBuffer,
			_In_   DWORD dwReceiveDataLength,
			_In_   DWORD dwLocalAddressLength,
			_In_   DWORD dwRemoteAddressLength,
			_Out_  LPSOCKADDR *LocalSockaddr,
			_Out_  LPINT LocalSockaddrLength,
			_Out_  LPSOCKADDR *RemoteSockaddr,
			_Out_  LPINT RemoteSockaddrLength);

		int recv_ex(
			_Inout_  LPWSABUF lpBuffers,
			_In_     DWORD dwBufferCount,
			_Out_    LPDWORD lpNumberOfBytesRecvd,
			_Inout_  LPDWORD lpFlags,
			_In_     LPWSAOVERLAPPED lpOverlapped,
			_In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
			);

		int recv_from_ex(
			_Inout_  LPWSABUF lpBuffers,
			_In_     DWORD dwBufferCount,
			_Out_    LPDWORD lpNumberOfBytesRecvd,
			_Inout_  LPDWORD lpFlags,
			_Out_    struct sockaddr *lpFrom,
			_Inout_  LPINT lpFromlen,
			_In_     LPWSAOVERLAPPED lpOverlapped,
			_In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

		int send_ex(
			_In_   LPWSABUF lpBuffers,
			_In_   DWORD dwBufferCount,
			_Out_  LPDWORD lpNumberOfBytesSent,
			_In_   DWORD dwFlags,
			_In_   LPWSAOVERLAPPED lpOverlapped,
			_In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

		int send_to_ex(
			_In_   LPWSABUF lpBuffers,
			_In_   DWORD dwBufferCount,
			_Out_  LPDWORD lpNumberOfBytesSent,
			_In_   DWORD dwFlags,
			_In_   const struct sockaddr *lpTo,
			_In_   int iToLen,
			_In_   LPWSAOVERLAPPED lpOverlapped,
			_In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

		int getsockname(
			_Out_    struct sockaddr *name,
			_Inout_  int *namelen);

		int getpeername(
			_Out_    struct sockaddr *name,
			_Inout_  int *namelen);

		int shutdown(
			_In_  int how);

		int setsockopt(
			_In_  int level,
			_In_  int optname,
			_In_  const char *optval,
			_In_  int optlen);

		int getsockopt(
			_In_     int level,
			_In_     int optname,
			_Out_    char *optval,
			_Inout_  int *optlen);

		int ioctlsocket(
			_In_     long cmd,
			_Inout_  u_long *argp);

		int close();

		void attach_iocp(
			_In_  iocp_t * iocp, 
			_In_  ULONG_PTR CompletionKey);

	public:
		static Windows::Networking::HostName ^ sockaddr_to_host_name(
			_In_  const struct sockaddr *name);

		static Platform::String ^ sockaddr_to_svc_name(
			_In_  const struct sockaddr *name);

		static int host_name_port_to_sockaddr(
			_In_  int af,
			_Out_    struct sockaddr *name,
			_Inout_  int *namelen, 
			Windows::Networking::HostName ^ host_name, 
			Platform::String ^ svc);

	private:
		static DWORD read_buffer(
			Windows::Storage::Streams::IBuffer ^ buffer, 
			_Inout_  LPWSABUF lpBuffers,
			_In_     DWORD dwBufferCount);

		static DWORD write_buffer(
			Windows::Storage::Streams::IBuffer ^ buffer, 
			_Inout_  LPWSABUF lpBuffers,
			_In_     DWORD dwBufferCount);

	private:
		int wait_action(
			Windows::Foundation::IAsyncAction ^ action);

		template <typename TResult, typename TProgress>
		int wait_operation2(
			Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^ operation, 
			TResult & result);

		void tcp_recv_some();

		void tcp_send_some();

		void handle_overlap_connect();

		void handle_overlap_accept();

		void handle_overlap_read();

		void handle_overlap_write();

		void accept_conn(
			_In_   socket_t * sock,
			_In_   PVOID lpOutputBuffer,
			_In_   DWORD dwReceiveDataLength,
			_In_   DWORD dwLocalAddressLength,
			_In_   DWORD dwRemoteAddressLength,
			_Out_  LPDWORD lpdwBytesReceived);

		void read_data(
			_Inout_  LPWSABUF lpBuffers,
			_In_     DWORD dwBufferCount,
			_Out_    LPDWORD lpNumberOfBytesRecvd);

		void write_data(
			_In_   LPWSABUF lpBuffers,
			_In_   DWORD dwBufferCount,
			_Out_  LPDWORD lpNumberOfBytesSent);

	private:
		enum StatusEnum
		{
			s_can_read = 1, 
			s_can_write = 2, 
			s_read_eof = 4, 
			s_write_eof = 8, 

			s_established = 16, 

			s_establish = s_established | s_can_read | s_can_write, 
		};

		enum FlagEnum
		{
			f_non_block, 
		};

	private:
		int af;
		int type;
		int protocol;
		Windows::Networking::Sockets::StreamSocket ^ stream_socket;
		Windows::Networking::Sockets::StreamSocketListener ^ stream_listener;
		Windows::Networking::Sockets::DatagramSocket ^ datagram_socket;
		std::deque<Windows::Networking::Sockets::StreamSocket ^> accept_sockets;
		boost::shared_ptr<iocp_t> iocp;
		ULONG_PTR lpCompletionKey;
		std::deque<overlap_task> read_tasks; // or accept tasks
		std::deque<overlap_task> write_tasks;
		std::deque<Windows::Storage::Streams::IBuffer ^> read_datas_;
		std::deque<Windows::Storage::Streams::IBuffer ^> write_datas_;
		Windows::Storage::Streams::DataReader ^ reader_;
		Windows::Storage::Streams::DataWriter ^ writer_;
		int flags_;
		int status_;
		size_t read_data_size_;
		size_t write_data_size_;
		size_t read_data_capacity_;
		size_t write_data_capacity_;
		bool connecting_;
		bool reading_;
		bool writing_;
		int ec_;
		std::recursive_mutex mutex_;
		std::condition_variable_any cond_;
	};

}
