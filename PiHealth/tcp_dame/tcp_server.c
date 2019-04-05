#include "../common/common.h"

int main(int argc, char *argv[]) {
	int server_listen, socketfd, port, pid;
	if (argc != 2) {
		printf("Usage: ./tcp_server port\n");
		exit(0);
	}

	port = atoi(argv[1]);

	if ((server_listen = socket_create(port)) < 0 ) {
		printf("Error creating server_listen socket");
		exit(1);
	}

	while(1) {
		// if ((socketfd = socket_accept(server_listen)) < 0 )
		// 	break;
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		if((socketfd = accept(server_listen, (struct sockaddr*) &client_addr, &len)) < 0)
			break;


		struct sockaddr_in peer;
		socklen_t peer_len = sizeof(struct sockaddr_in);
		bzero(&peer, sizeof(struct sockaddr_in));
		getpeername(socketfd, (struct sockaddr *)&peer, &peer_len);
		char buff_peer[64] = {'\0'};
		if (inet_ntop(AF_INET, (void*)&peer.sin_addr, buff_peer, 63)) {
			printf("peer ip: %s    peer port:%d\n", buff_peer, ntohs(peer.sin_port));
		}

		printf("peer ip1 = %s ", inet_ntoa(client_addr.sin_addr));
		printf("   port2 = %d\n", htons(client_addr.sin_port));


		fflush(stdout);

		if ((pid = fork()) < 0)
			printf("Error forking child process");
		if (pid == 0) {
			close(server_listen);
			char* buffer = (char *)malloc(MAX_SIZE*sizeof(char));
			recv_data(socketfd, buffer, MAX_SIZE);
			printf("%s\n", buffer);
			close(socketfd);
			exit(0);
		}

	}

}