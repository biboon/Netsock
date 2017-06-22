#include "netsock.h"

#include "log.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif


/**
 * Create a non blocking udp/tcp socket and connect it
 * @param host Address to connect to
 * @param service Port to connect to
 * @param flags Flags passed to the addrinfo structure
 * @param family Flag to specify IPv4 or IPv6
 * @param socktype Type datagram or stream socket
 * @param protocol Flag for specifying either TCP or UDP
 * @return The file descriptor for using the socket or -1 if error
 */
static socket_t
netsock_connect(const char *host, const char *service,
	int flags, int family, int socktype, int protocol)
{
	/* Get address info */
	const struct addrinfo hints = {
		.ai_flags = flags,
		.ai_family = family,
		.ai_socktype = socktype,
		.ai_protocol = protocol
	};
	struct addrinfo *result;
	int status = getaddrinfo(host, service, &hints, &result);
	if (status != 0) {
		log_error("getaddrinfo: %s", gai_strerror(status));
		return -1;
	}
	/* Go through the linked list and try to connect */
	socket_t fd;
	struct addrinfo *rp;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (-1 == fd) continue;
		if (-1 != connect(fd, rp->ai_addr, rp->ai_addrlen)) break;
		close(fd);
	}
	freeaddrinfo(result);
	if (rp == NULL) {
		log_perror("connect");
		return -1;
	}
	return fd;
}

/**
 * Create a non blocking tcp stream socket and connect it
 * @param host Address to connect to
 * @param service Port to connect to
 * @return The file descriptor for reading and writing on the socket
 */
socket_t
netsock_connect_stream(const char *host, const char *service)
{
	return netsock_connect(host, service,
		AI_ADDRCONFIG | AI_NUMERICHOST | AI_NUMERICSERV,
		AF_INET, SOCK_STREAM, 0);
}

/**
 * Create a non blocking udp datagram socket and connect it
 * @param host Address to connect to
 * @param service Port to connect to
 * @return The file descriptor for reading and writing on the socket
 */
socket_t
netsock_connect_dgram(const char *host, const char *service)
{
	return netsock_connect(host, service,
		AI_ADDRCONFIG | AI_NUMERICHOST | AI_NUMERICSERV,
		AF_INET, SOCK_DGRAM, 0);
}


/**
 * Create a non blocking udp/tcp socket and bind it to local port
 * @param service Port to bind to
 * @param flags Flags passed to the addrinfo structure
 * @param family Flag to specify IPv4 or IPv6
 * @param socktype Type datagram or stream socket
 * @param protocol Flag for specifying either TCP or UDP
 * @return The file descriptor for reading and writing on the socket
 */
static socket_t
netsock_bind(const char *service, int flags, int family, int socktype, int protocol)
{
	/* Get address info */
	const struct addrinfo hints = {
		.ai_flags = flags,
		.ai_family = family,
		.ai_socktype = socktype,
		.ai_protocol = protocol
	};
	struct addrinfo *result;
	int status = getaddrinfo(NULL, service, &hints, &result);
	if (status != 0) {
		log_error("getaddrinfo: %s", gai_strerror(status));
		return -1;
	}
	/* Go through the linked list and try to bind */
	socket_t fd;
	struct addrinfo *rp;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (-1 == fd) continue;
		if (-1 != bind(fd, rp->ai_addr, rp->ai_addrlen)) break;
		close(fd);
	}
	freeaddrinfo(result);
	if (rp != NULL) return fd;
	log_perror("bind");
	return -1;
}

/**
 * Create a non blocking tcp stream socket and bind it to local port
 * @param service Port to connect to
 * @return The file descriptor for reading and writing on the socket
 */
socket_t
netsock_bind_stream(const char *service)
{
	socket_t socket = netsock_bind(service, AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE,
		AF_INET, SOCK_STREAM, 0);
	if (socket < 0) return -1;
	/* Set SO_REUSEADDR option */
	int opt = 1;
	if (-1 == setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		log_perror("setsockopt");
		netsock_close(socket);
		return -1;
	}
	/* Listen for new connections */
	if (-1 == listen(socket, 5)) {
		log_perror("listen");
		netsock_close(socket);
		return -1;
	}
	log_debug("Listening on port %s (socket: %d)", service, socket);
	return socket;
}

/**
 * Calls accept on the defined socket.
 * @param socket A socket bound and listening for connections
 * @return A new socket used to communicate with the new client
 */
socket_t netsock_accept(socket_t socket)
{
	log_debug("Accepting new connection");
	return accept(socket, NULL, NULL);
}

/**
 * Calls setsockopt on the defined socket to set a timeout value.
 * @param socket The socket file descriptor to apply the modification on
 * @param millis The number of milliseconds to wait before timing out
 * @param type Timeout on receive or send direction
 */
static int netsock_set_timeout(socket_t socket, int millis, int type)
{
	if (millis < 0) return -1;
	const struct timeval timeout = {
		.tv_sec = millis / 1000,
		.tv_usec = (millis % 1000) * 1000
	};
	if (0 != setsockopt(socket, SOL_SOCKET, type, &timeout, sizeof(timeout))) {
		log_perror("setsockopt");
		return -1;
	}
	return 0;
}

inline int netsock_set_timeout_recv(socket_t socket, int millis)
{
	return netsock_set_timeout(socket, millis, SO_RCVTIMEO);
}

inline int netsock_set_timeout_send(socket_t socket, int millis)
{
	return netsock_set_timeout(socket, millis, SO_SNDTIMEO);
}


/**
 * This function is intended to be used on datagram sockets. It performs a
 * single call to read and saves in a buffer the data read.
 * @param socket The file descriptor for reading on the sockets
 * @param buf The memory location to save data in
 * @param count The maximum amount of data to read
 * @return Returns the amount of bytes read or -1 on error.
 */
ssize_t netsock_read_dgram(socket_t socket, void *buf, size_t count)
{
	ssize_t size = recv(socket, buf, count, 0);
	if (likely(size >= 0)) {
		log_all("read dgram %d: %zd/%zdB", socket, size, count);
		return size;
	}
	log_perror("recv (%d)", socket);
	return -1;
}


/**
 * This function is intended to be used on stream sockets. It performs at least
 * one call to read and saves in a buffer the data read. It will read exactly
 * the amount of bytes specified or less if a timeout occured.
 * @param socket The file descriptor for reading on the sockets
 * @param buf The memory location to save data in
 * @param count The amount of data to read
 * @return Returns the amount of bytes read or -1 if on error.
 */
ssize_t netsock_read_stream(socket_t socket, void *buf, size_t count)
{
	size_t left = count;
	do {
		ssize_t size = recv(socket, buf, left, 0);
		if (unlikely(size < 0)) {
			log_perror("recv (%d)", socket);
			return -1;
		}
		buf += size;
		left -= size;
	} while (left);
	log_all("read stream %d: %zdB", socket, count);
	return count;
}


/**
 * If the socket is of type datagram, this function will perform a single call
 * to write, consequently the amount of bytes should not be too large. Otherwise,
 * if the socket is a stream one, it may call several times write and the amount
 * of bytes to write can be larger.
 * @param socket The file descriptor for reading on the sockets
 * @param buf The memory location to read data from
 * @param count The amount of data to write
 * @return Returns the amount of bytes written or -1 if an error occured during operation.
 */
ssize_t netsock_write(socket_t socket, const void *buf, size_t count)
{
	size_t left = count;
	do {
		ssize_t size = send(socket, buf, left, 0);
		if (unlikely(size < 0)) {
			log_perror("send (%d)", socket);
			return -1;
		}
		buf += size;
		left -= size;
	} while (left);
	log_all("write %d: %zdB", socket, count);
	return count;
}


/**
 * Close a socket given by its file descriptor
 * @param socket The file descriptor for closing the socket
 */
void netsock_close(socket_t socket) {
	log_trace("Closing %d", socket);
	shutdown(socket, SHUT_RDWR);
	close(socket);
}