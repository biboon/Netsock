#include "netsock.h"

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <host> <port>\n", argv[0]);
		return -1;
	}
	
	ns_start();
	netsock_t socket;

	/* Send data */
	socket = ns_client(argv[1], argv[2], NS_FAMILY_IPV4, NS_TYPE_TCP, 0);
	
	char buffer[32];
	for (int i = 0; i < 3; ++i) {
		if (fgets(buffer, sizeof buffer, stdin) == NULL) break;
		ns_send(socket, buffer, strlen(buffer));
	}
	
	ns_shutdown(socket, NS_SHUT_RDWR);
	ns_close(socket);
	
	getchar();

	/* Receive data */
	socket = ns_listener("0.0.0.0", "11234", NS_FAMILY_IPV4, NS_TYPE_TCP, 0);
	netsock_t client = ns_accept(socket);
	
	for (int i = 0; i < 3; ++i) {
		memset(buffer, 0, sizeof buffer);
		ns_recv_all(client, buffer, 12);
		printf("client: \"%s\"\n", buffer);
	}

	ns_shutdown(client, NS_SHUT_RDWR);
	ns_close(client);
	ns_close(socket);
	
	ns_end();
	
	return 0;
}