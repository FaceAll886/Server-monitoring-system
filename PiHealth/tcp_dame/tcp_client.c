#include "../common/common.h"


int main(int argc, char *argv[]) {
	int sock_client, socket_fd;

	if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket create error");
		return -1;
	}
	
	char* ip_addr = argv[1];
	int port = atoi(argv[2]);
	
	while (1) {
		socket_fd = socket_connect(port, ip_addr);
		char* buff[MAX_SIZE];
		scanf("%ss", &buff);
		int num = send(socket_fd, buff, MAX_SIZE, 0);
		close(socket_fd);
		close(sock_client);
	}

	return 0;

}
