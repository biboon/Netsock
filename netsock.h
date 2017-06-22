#ifndef __NETSOCK_H
#define __NETSOCK_H

#include <stdlib.h>

#if ( defined(__unix__) )
#include <unistd.h>
#include <sys/socket.h>

typedef int socket_t;

#define  NETSOCK_ERROR                  -1
#define  NETSOCK_INVALID                -1
#define  NETSOCK_SHUT_RD                SHUT_RD
#define  NETSOCK_SHUT_WR                SHUT_WR
#define  NETSOCK_SHUT_RDWR              SHUT_RDWR

#define  netsock_close(socket)          close(socket)

#elif ( defined(_WIN32) || defined(_WIN64) )
#include <winsock2.h>

typedef SOCKET socket_t;

#define  NETSOCK_ERROR                  SOCKET_ERROR
#define  NETSOCK_INVALID                INVALID_SOCKET
#define  NETSOCK_SHUT_RD                SD_RECEIVE
#define  NETSOCK_SHUT_WR                SD_SEND
#define  NETSOCK_SHUT_RDWR              SD_BOTH

#define  netsock_close(socket)          closesocket(socket)

#endif

#define  netsock_shutdown(socket, how)  shutdown(socket, how)

int      netsock_start(void);
int      netsock_end(void);

socket_t netsock_connect_stream(const char *host, const char *service);
socket_t netsock_connect_dgram(const char *host, const char *service);
socket_t netsock_bind_stream(const char *service);
socket_t netsock_accept(socket_t socket);

int      netsock_set_timeout_recv(socket_t socket, int timeout);
int      netsock_set_timeout_send(socket_t socket, int timeout);

ssize_t  netsock_read_datagm(socket_t socket, void *buf, size_t size);
ssize_t  netsock_read_stream(socket_t socket, void *buf, size_t size);
ssize_t  netsock_write(socket_t socket, const void *buf, size_t size);

#endif /* __NETSOCK_H */
