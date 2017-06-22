#include "log.h"
#include "netsock.h"

#include <stdio.h>


int main(int argc, char *argv[])
{
	log_start(NULL);
	if (argc < 3) {
		log_fatal("Usage: %s <host> <port>", argv[0]);
		return -1;
	}
	socket_t socket;
	netsock_start();
	/* Send data */
	socket = netsock_connect_stream(argv[1], argv[2]);
	char buffer[32];
	for (int i = 0; i < 3; ++i) {
		if (fgets(buffer, sizeof buffer, stdin) == NULL) break;
		netsock_write(socket, buffer, strlen(buffer));
	}
	netsock_shutdown(socket, NETSOCK_SHUT_RDWR);
	netsock_close(socket);
	/* Receive data */
	getchar();
	socket = netsock_bind_stream("11234");
	socket_t client = netsock_accept(socket);
	for (int i = 0; i < 3; ++i) {
		memset(buffer, 0, sizeof buffer);
		netsock_read_stream(client, buffer, 12);
		printf("client: \"%s\"\n", buffer);
	}
	netsock_close(socket);
	netsock_close(client);
	netsock_end();
	log_end();
	return 0;
}