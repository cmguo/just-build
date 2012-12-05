// SocketEmulation.cpp

#include "WsaContext.h"

#include <deque>
#include <mutex>
#include <condition_variable>

#include <assert.h>

namespace SocketEmulation
{

	struct iocp_t
		: wsa_handle_t<iocp_t, 2>
	{
		void push(
			ULONG_PTR lpCompletionKey, 
			LPOVERLAPPED lpOverlapped, 
			DWORD dwNumberOfBytesTransferred);

		BOOL pop(
			_Out_  LPDWORD lpNumberOfBytes,
			_Out_  PULONG_PTR lpCompletionKey,
			_Out_  LPOVERLAPPED *lpOverlapped,
			_In_   DWORD dwMilliseconds);

		BOOL close();

		std::mutex mutex_;
		std::condition_variable cond_;
		std::deque<OVERLAPPED_ENTRY> overlaps_;
	};

}
