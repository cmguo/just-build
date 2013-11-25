// boost_config.h

#define __USE_W32_SOCKETS

#define __WSAFDIsSet(fd, set) FD_ISSET(fd, set)
