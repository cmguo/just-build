// boost_config.h

// asio disable epoll
#define BOOST_ASIO_DISABLE_EPOLL

#include <unistd.h>
#undef _POSIX_SHARED_MEMORY_OBJECTS

#define CLOCK_MONOTONIC TIMER_ABSTIME

#define get_nprocs() 1
