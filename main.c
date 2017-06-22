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
	socket_t socket = netsock_connect_stream(argv[1], argv[2]);
	char buffer[32];
	for (int i = 0; i < 12; ++i) {
		if (fgets(buffer, sizeof buffer, stdin) == NULL) break;
		netsock_write(socket, buffer, strlen(buffer));
	}
	netsock_shutdown(socket, NETSOCK_SHUT_RDWR);
	netsock_close(socket);
	return 0;
}