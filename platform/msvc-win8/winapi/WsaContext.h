// WsaContext.h

#pragma once

#include "SocketEmulation.h"
#include "ThreadEmulation.h"
using namespace ThreadEmulation;

#include <vector>
#include <deque>
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <condition_variable>

#include <assert.h>

namespace SocketEmulation
{

	struct tls_data_t
	{
		tls_data_t()
		{
			hostent = new struct hostent;
			hostent->h_name = NULL;
			hostent->h_aliases = NULL;
			hostent->h_addr_list = hostent_addr_list;
			memset(hostent_addr_list, 0, sizeof(hostent_addr_list));
		}

		~tls_data_t()
		{
			delete hostent;
		}

		int wait_action(
			Windows::Foundation::IAsyncAction ^ action)
		{
			std::unique_lock<std::recursive_mutex> lc(mutex_);
			action->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler([this](Windows::Foundation::IAsyncAction^, Windows::Foundation::AsyncStatus) {
				std::unique_lock<std::recursive_mutex> lc(mutex_);
				cond_.notify_all();
			});
			cond_.wait(lc, [=]() {
				return action->Status  != Windows::Foundation::AsyncStatus::Started;
			});
			Windows::Foundation::AsyncStatus status = action->Status;
			Windows::Foundation::HResult result = action->ErrorCode;
			WSASetLastError(SCODE_CODE(result.Value));
			return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
		}

		template <typename TResult>
		int wait_operation(
			Windows::Foundation::IAsyncOperation<TResult>  ^ operation, 
			TResult & result)
		{
			std::unique_lock<std::recursive_mutex> lc(mutex_);
			operation->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<TResult>([this](
				Windows::Foundation::IAsyncOperation<TResult>  ^, Windows::Foundation::AsyncStatus) {
					std::unique_lock<std::recursive_mutex> lc(mutex_);
					cond_.notify_all();
			});
			cond_.wait(lc, [=]() {
				return operation->Status  != Windows::Foundation::AsyncStatus::Started;
			});
			Windows::Foundation::AsyncStatus status = operation->Status;
			WSASetLastError(SCODE_CODE(operation->ErrorCode.Value));
			result = operation->GetResults();
			return status == Windows::Foundation::AsyncStatus::Completed ? 0 : SOCKET_ERROR;
		}

		struct hostent * hostent;
		char * hostent_addr_list[4];
		char hostent_addr_chars[256];
		std::recursive_mutex mutex_;
		std::condition_variable_any cond_;
	};

	struct wsa_handle
		: public boost::enable_shared_from_this<wsa_handle>
	{
		typedef boost::shared_ptr<wsa_handle> pointer_t;

		size_t index;
		size_t cls; // 1 - socket, 2 - iocp
	};

	template <typename T, size_t C>
	struct wsa_handle_t
		: wsa_handle
	{
		static size_t const CLS = C;
	};

	struct wsa_context
	{
		wsa_context()
		{
			tls_data_index = TlsAlloc();
			handles_.push_back(wsa_handle::pointer_t()); // 0 ÊÇÎÞÐ§¾ä±ú
		}

		~wsa_context()
		{
			TlsFree(tls_data_index);
		}

		tls_data_t * tls_data()
		{
			tls_data_t * d = (tls_data_t *)TlsGetValue(tls_data_index);
			if (d == NULL) {
				d = new tls_data_t;
				TlsSetValue(tls_data_index, d);
			}
			return d;
		}

		template <typename handle_t>
		handle_t * alloc()
		{
			std::unique_lock<std::mutex> lc(mutex_);
			wsa_handle::pointer_t handle(new handle_t);
				handle->cls = handle_t::CLS;
			if (free_indexs_.empty()) {
				handle->index = handles_.size();
				handles_.push_back(handle);
			} else {
				handle->index = free_indexs_.front();
				free_indexs_.pop_front();
				assert(handles_[handle->index] == NULL);
				handles_[handle->index] = handle;
			}
			return (handle_t *)handle.get();
		}

		template <typename handle_t>
		void free(
			handle_t * handle)
		{
			std::unique_lock<std::mutex> lc(mutex_);
			assert(handles_[handle->index]);
			free_indexs_.push_back(handle->index);
			handles_[handle->index].reset();
		}

		template <typename handle_t>
		handle_t * get(
			size_t index)
		{
			std::unique_lock<std::mutex> lc(mutex_);
			assert(handles_[index]);
			handle_t * handle = (handle_t *)handles_[index].get();
			assert(handle->cls == handle_t::CLS);
			return handle;
		}

	private:
		std::mutex mutex_;
		std::vector<wsa_handle::pointer_t> handles_;
		std::deque<size_t> free_indexs_;
		DWORD tls_data_index;
	};

	inline wsa_context & g_wsa_context()
	{
		static wsa_context context;
		return context;
	}

}
