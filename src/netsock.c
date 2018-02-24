#include "netsock.h"

#include <stdio.h>
#include <string.h>

#if defined(__linux__)

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#elif defined(_WIN32) || defined(_WIN64)

#include <ws2tcpip.h>

#endif


/**
 * Initializes the library and its underlying components
 * @return 0 or NS_ERROR
 */
int ns_start(void)
{
#if defined(__linux__)

	return 0;

#elif defined(_WIN32) || defined(_WIN64)

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	return WSAStartup(wVersionRequested, &wsaData) == 0 ? 0 : NS_ERROR;

#endif
}

/**
 * Cleans up the library and its underlying components
 * @return 0 or NS_ERROR
 */
int ns_end(void)
{
#if defined(__linux__)

	return 0;

#elif defined(_WIN32) || defined(_WIN64)

	return WSACleanup() == 0 ? 0 : NS_ERROR;

#endif
}


/**
 * Create a TCP or UDP server socket for listening
 * @param hostname A resolvable hostname to bind to
 * @param port The port to bind to. Numeric or service name
 * @param family Flag to specify IPv4, IPv6 or both
 * @param flags Flags passed to socket
 * @return The file descriptor for using the socket or NS_INVALID if error
 */
netsock_t ns_listener(const char *hostname, const char *port, int family, int type, int flags)
{
	struct addrinfo hints, *result, *rp;
	netsock_t sfd;
	int status;

	memset(&hints, 0, sizeof(struct addrinfo));

	switch (family)
	{
		case NS_FAMILY_IPV4: hints.ai_family = AF_INET;   break;
		case NS_FAMILY_IPV6: hints.ai_family = AF_INET6;  break;
		case NS_FAMILY_ANY : hints.ai_family = AF_UNSPEC; break;
		default: return NS_INVALID;
	}

	switch (type)
	{
		case NS_TYPE_TCP: hints.ai_socktype = SOCK_STREAM; break;
		case NS_TYPE_UDP: hints.ai_socktype = SOCK_DGRAM;  break;
		default: return NS_INVALID;
	}

	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(hostname, port, &hints, &result);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return NS_INVALID;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype | flags, rp->ai_protocol);
		if (sfd == NS_INVALID) continue;

#if defined(__linux__)
		status = bind(sfd, rp->ai_addr, rp->ai_addrlen);
#elif defined(_WIN32) || defined(_WIN64)
		status = bind(sfd, rp->ai_addr, (int) rp->ai_addrlen);
#endif

		if (status == 0)
		{
			if (type == NS_TYPE_UDP) break;
			if (listen(sfd, NS_BACKLOG) == 0) break;
		}

		ns_close(sfd);
	}

	freeaddrinfo(result);

	return rp != NULL ? sfd : NS_INVALID;
}


/**
 * Create and connect a new TCP/IP socket
 * @param hostname A resolvable hostname
 * @param port The port, numeric or as a service name
 * @param family Flag to specify IPv4 or IPv6 or both
 * @param flags Flags passed to socket
 * @return The file descriptor for using the socket or NS_INVALID if error
 */
netsock_t ns_client(const char *hostname, const char *port, int family, int type, int flags)
{
	struct addrinfo hints, *result, *rp;
	netsock_t sfd;
	int status;

	memset(&hints, 0, sizeof(struct addrinfo));

	switch (family)
	{
		case NS_FAMILY_IPV4: hints.ai_family = AF_INET;   break;
		case NS_FAMILY_IPV6: hints.ai_family = AF_INET6;  break;
		case NS_FAMILY_ANY : hints.ai_family = AF_UNSPEC; break;
		default: return NS_INVALID;
	}

	switch (type)
	{
		case NS_TYPE_TCP: hints.ai_socktype = SOCK_STREAM; break;
		case NS_TYPE_UDP: hints.ai_socktype = SOCK_DGRAM;  break;
		default: return NS_INVALID;
	}

	status = getaddrinfo(hostname, port, &hints, &result);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return NS_INVALID;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype | flags, rp->ai_protocol);
		if (sfd == NS_INVALID) continue;

#if defined(__linux__)
		status = connect(sfd, rp->ai_addr, rp->ai_addrlen);
#elif defined(_WIN32) || defined(_WIN64)
		status = connect(sfd, rp->ai_addr, (int) rp->ai_addrlen);
#endif

		if (status == 0) break;
		ns_close(sfd);
	}

	freeaddrinfo(result);

	return rp != NULL ? sfd : NS_INVALID;
}


/**
 * Receive a datagram and stores the remote endpoint info in hostname and port
 * @param sfd The socket to use for receiving the datagram
 * @param buf Points to a buffer where the data will be stored
 * @param len The size of the buffer
 * @param flags Specifies the type of transmission
 * @param hostname The remote hostname
 * @param hlen The size of the buffer for the hostname
 * @param port The remote port
 * @param plen The size of the buffer for the port
 */
int ns_dgram_recvfrom(netsock_t sfd, void *buf, size_t len, int flags,
	char *hostname, size_t hlen, char *port, size_t plen)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	int size;
	int status;

	memset(&addr, 0, sizeof(struct sockaddr_storage));

#if defined(__linux__)
	size = recvfrom(sfd, buf, len, flags, (struct sockaddr *) &addr, &addrlen);
#elif defined(_WIN32) || defined(_WIN64)
	size = recvfrom(sfd, buf, (int) len, flags, (struct sockaddr *) &addr, &addrlen);
#endif

	if (size == NS_ERROR) return NS_ERROR;

#if defined(__linux__)
	status = getnameinfo((struct sockaddr *) &addr, addrlen, hostname,
		(socklen_t) hlen, port, (socklen_t) plen, NI_NUMERICHOST | NI_NUMERICSERV);
#elif defined(_WIN32) || defined(_WIN64)
	status = getnameinfo((struct sockaddr *) &addr, addrlen,
		hostname, (DWORD) hlen, port, (DWORD) plen, NI_NUMERICHOST | NI_NUMERICSERV);
#endif

	if (status != 0)
	{
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
		return NS_ERROR;
	}

	return size;
}

/**
 * Send a datagram to a specific endpoint
 * @param sfd The socket to use for sending the datagram
 * @param buf Points to a buffer containing the data to be sent
 * @param len The length of the message
 * @param flags Specifies the type of transmission
 * @param hostname A resolvable hostname
 * @param port The port, numeric or as a service name
 * @return Returns the number of bytes sent or NS_ERROR on error
 */
int ns_dgram_sendto(netsock_t sfd, const void *buf, size_t len, int flags,
	const char *hostname, const char *port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	struct addrinfo hints, *result, *rp;
	int size;
	int status;

	memset(&hints, 0, sizeof(struct addrinfo));

	status = getsockname(sfd, (struct sockaddr*)&addr, &addrlen);
	if (status != 0)
	{
		perror("getsockname");
		return NS_ERROR;
	}

	hints.ai_family = addr.ss_family;
	hints.ai_socktype = SOCK_DGRAM;

	status = getaddrinfo(hostname, port, &hints, &result);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return NS_ERROR;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
#if defined(__linux__)
		size = sendto(sfd, buf, len, flags, rp->ai_addr, rp->ai_addrlen);
#elif defined(_WIN32) || defined(_WIN64)
		size = sendto(sfd, buf, (int) len, flags, rp->ai_addr, (int) rp->ai_addrlen);
#endif

		if (size != NS_ERROR) break;
	}

	freeaddrinfo(result);

	return rp != NULL ? size : NS_ERROR;
}


int ns_recv_all(netsock_t socket, void *buf, size_t len)
{
	char *buffer = buf;
	int received = 0;

	while (received < len)
	{
		int status = ns_recv(socket, buffer + received, len - received);
		if (status == NS_ERROR) return NS_ERROR;
		if (status == 0) break;
		received += status;
	}

	return received;
}

int ns_send_all(netsock_t socket, const void *buf, size_t len)
{
	const char *buffer = buf;
	int sent = 0;

	while (sent < len)
	{
		int status = ns_send(socket, buffer + sent, len - sent);
		if (status == NS_ERROR) return NS_ERROR;
		if (status == 0) break;
		sent += status;
	}

	return sent;
}
