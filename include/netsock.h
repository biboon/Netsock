#ifndef __NS_H
#define __NS_H


#if defined(__linux__)

#include <stddef.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#elif defined(_WIN32) || defined(_WIN64)

#include <winsock2.h>
#include <windows.h>

#endif


#if defined(__linux__)

#define netsock_t                   int
#define NS_ERROR                    -1
#define NS_INVALID                  -1
#define NS_SHUT_RD                  SHUT_RD
#define NS_SHUT_WR                  SHUT_WR
#define NS_SHUT_RDWR                SHUT_RDWR

#elif defined(_WIN32) || defined(_WIN64)

#define netsock_t                   SOCKET
#define NS_ERROR                    SOCKET_ERROR
#define NS_INVALID                  INVALID_SOCKET
#define NS_SHUT_RD                  SD_RECEIVE
#define NS_SHUT_WR                  SD_SEND
#define NS_SHUT_RDWR                SD_BOTH

#endif


#define NS_BACKLOG     5
#define NS_TYPE_TCP    0x01
#define NS_TYPE_UDP    0x02
#define NS_FAMILY_IPV4 0x04
#define NS_FAMILY_IPV6 0x08
#define NS_FAMILY_ANY  (NS_FAMILY_IPV4 | NS_FAMILY_IPV6)


#if defined(__linux__)

#define ns_recv(s, buf, len)        recv(s, buf, len, 0)
#define ns_send(s, buf, len)        send(s, buf, len, 0)

#define ns_accept(socket)           accept(socket, NULL, NULL)
#define ns_shutdown(socket, how)    shutdown(socket, how)
#define ns_close(s)                 close(s)

#define ns_poll(fds, nfds, timeout) poll(fds, nfds, timeout)
#define ns_select(nfds, rfds, wfds, efds, timeout) \
	select(nfds, rfds, wfds, efds, timeout)

#elif defined(_WIN32) || defined(_WIN64)

#define ns_recv(s, buf, len)        recv(s, (      char *)(buf), (int)(len), 0)
#define ns_send(s, buf, len)        send(s, (const char *)(buf), (int)(len), 0)

#define ns_accept(socket)           accept(socket, NULL, NULL)
#define ns_shutdown(socket, how)    shutdown(socket, how)
#define ns_close(socket)            closesocket(socket)

#define ns_poll(fds, nfds, timeout) WSAPoll(fds, nfds, timeout)
#define ns_select(nfds, rfds, wfds, efds, timeout) \
	select(0, rfds, wfds, efds, timeout)

#endif


#ifdef __cplusplus
extern "C" {
#endif

int ns_start(void);
int ns_end(void);

netsock_t ns_listener(const char *hostname, const char *port, int family, int type, int flags);
netsock_t ns_client(const char *hostname, const char *port, int family, int type, int flags);

int ns_dgram_recvfrom(netsock_t sfd, void *buf, size_t len, int flags,
	char *hostname, size_t hlen, char *port, size_t plen);
int ns_dgram_sendto(netsock_t sfd, const void *buf, size_t len, int flags,
	const char *hostname, const char *port);

int ns_recv_all(netsock_t socket, void *buf, size_t len);
int ns_send_all(netsock_t socket, const void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __NS_H */
