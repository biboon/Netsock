#ifndef __NETSOCK_H__
#define __NETSOCK_H__

#if ( defined(__unix__) )

typedef int socket_t;
//#define netsock_close(socket_t)  close(socket_t)
#define NETSOCK_SHUT_RD          SHUT_RD
#define NETSOCK_SHUT_WR          SHUT_WR
#define NETSOCK_SHUT_RDWR        SHUT_RDWR
#define NETSOCK_ERROR            -1

#elif ( defined(_WIN32) || defined(_WIN64) )

typedef SOCKET socket_t;
#define netsock_close(socket_t)  closesocket(socket_t)
#define NETSOCK_SHUT_RD          SD_RECEIVE
#define NETSOCK_SHUT_WR          SD_SEND
#define NETSOCK_SHUT_RDWR        SD_BOTH
#define NETSOCK_ERROR            SOCKET_ERROR

#endif


#include <stdlib.h>

socket_t netsock_connect_stream(const char *host, const char *service);
socket_t netsock_connect_dgram(const char *host, const char *service);
socket_t netsock_bind_stream(const char *service);
socket_t netsock_accept(socket_t socket);
int netsock_set_timeout_recv(socket_t socket, int millis);
int netsock_set_timeout_send(socket_t socket, int millis);
ssize_t netsock_read_dgram(socket_t socket, void *buf, size_t count);
ssize_t netsock_read_stream(socket_t socket, void *buf, size_t count);
ssize_t netsock_write(socket_t socket, const void *buf, size_t count);
void netsock_close(socket_t socket);

#endif
