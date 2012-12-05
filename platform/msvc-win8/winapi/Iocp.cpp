// Iocp.cpp

#include <Windows.h>

#undef WINAPI
#define WINAPI

#include "Iocp.h"

namespace SocketEmulation
{

	class set_last_error
	{
	public:
		set_last_error()
			: ec_(0)
		{
		}
		
		void set(
			int ec)
		{
			ec_ = ec;
		}

		~set_last_error()
		{
			SetLastError(ec_);
		}

	private:
		int ec_;
	};

	void iocp_t::push(
		ULONG_PTR lpCompletionKey, 
		LPOVERLAPPED lpOverlapped, 
		DWORD dwNumberOfBytesTransferred)
	{
		std::unique_lock<std::mutex> lc(mutex_);
		OVERLAPPED_ENTRY entry = {lpCompletionKey, lpOverlapped, 0, dwNumberOfBytesTransferred};
		overlaps_.push_back(entry);
		cond_.notify_one();
	}

	BOOL iocp_t::pop(
		_Out_  LPDWORD lpNumberOfBytes,
		_Out_  PULONG_PTR lpCompletionKey,
		_Out_  LPOVERLAPPED *lpOverlapped,
		_In_   DWORD dwMilliseconds)
	{
		set_last_error le;
		std::unique_lock<std::mutex> lc(mutex_);
		if (!cond_.wait_for(lc, std::chrono::milliseconds(dwMilliseconds), [this]() {
			return !overlaps_.empty();
		})) {
			le.set(WAIT_TIMEOUT);
			return FALSE;
		}
		OVERLAPPED_ENTRY entry = overlaps_.front();
		overlaps_.pop_front();
		lc.unlock();
		*lpNumberOfBytes = entry.dwNumberOfBytesTransferred;
		*lpCompletionKey = entry.lpCompletionKey;
		*lpOverlapped = entry.lpOverlapped;
		return TRUE;
	}

	BOOL iocp_t::close()
	{
		std::unique_lock<std::mutex> lc(mutex_);
		overlaps_.clear();
		return TRUE;
	}

}
