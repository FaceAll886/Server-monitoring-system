#include "client.h"

char *config = "./script/PiHealth.conf";

void send_file(int sockfd, char* filename){
	FILE* fd = NULL;
	char data[MAX_SIZE];
	size_t num_read;
	fd = fopen(filename, "r");
	if (!fd) {
		printf("File open error\n");
	} else {
		printf("File open sucess\n");
		while (1) {
			num_read = fread(data, 1, MAX_SIZE, fd);
			if (send(sockfd, data, num_read, 0) < 0) 
				printf("error in sending file\n");
			if (num_read == 0) {
				break;
			}
		}
		fclose(fd);
	}
}


int check_log_file(int reqcode, char* log) {
	char log_dir[MAX_SIZE] = {0};
	if (get_conf_value(config, "LogDir",log_dir) < 0) {
		printf("log_dir error\n");
		exit(1);
	}
	strcpy(log, log_dir);
	if (reqcode == 100) {
		strcat(log, "/cpu.log");
	} else if (reqcode == 101) {
		strcat(log, "/mem.log");
	} else if (reqcode == 102) {
		strcat(log, "/disk.log");
	} else if (reqcode == 103) {
		strcat(log, "/proc.log");
	} else if (reqcode == 104) {
		strcat(log, "/sysinfo.log");
	} else {
		strcat(log, "/users.log");
	}

	if ((access(log, F_OK)) == -1) {
		printf("file : %s not found\n", log);
		return -1;
	}
	return 0;
}

int client_process(int master_request){
	int sock_data;
	char log[MAX_SIZE];

	while (1) {
		int rq = recv_response(master_request);
		if (rq <= 0)
		{
			return 1;
		}
		printf("Recieved REQUEST code %d\n", rq);
		if (rq < 100 || rq > 105) {
			printf("request error\n");
			return 1;
		}
		if (check_log_file(rq, log) < 0)
		{
			send_response(master_request, rq + 300);
			printf("Log file not found, send RESPONE %d\n ", rq + 300);
			//exit(1);
			continue;
		}

		send_response(master_request, rq + 100);	
		printf("RESPONE Code %d is sent\n", rq + 100);

		if ((recv_response(master_request)) == 1) {
			printf("ACK 1 recieved, Connect to MAster\n");
			if ((sock_data = client_start_data_conn(master_request)) < 0)  {
				close(sock_data); //
				exit(1);
			}
			printf("%s\n", "Connection is OK , start sending file");
			send_file(sock_data, log);
			printf("File sent to Master\n ");
			close(sock_data);
			return 0;
		}
	}
	return 0;
}

int client_start_data_conn(int master_request) {
	char buf[1024];
	int sock_data;

	struct sockaddr_in master_addr;

	memset(&master_addr, 0, sizeof(master_addr));
	socklen_t len = sizeof(master_addr);
	getpeername(master_request, (struct sockaddr*)&master_addr, &len);
	inet_ntop(AF_INET, &master_addr.sin_addr, buf, sizeof(buf));

	if ((sock_data = socket_connect(MASTER_PORT, buf)) < 0)
		return -1;
	return sock_data;
}


int main(int argc, char *argv[]) {
	int client_listen, master_request, port, pid;
	char port_t[5] = {0}; 
	if (argc != 1) {
		printf("Usage: ./client\n");
		exit(0);
	}

	get_conf_value(config, "ClientPort",port_t);
	port = atoi(port_t);


	if ((client_listen = socket_create(port)) < 0 ) {
		printf("Error creating client_listen socket");
		exit(1);
	}
	printf("Create listen socket on port %d\n", port);
	
	while (1) {
	master_request = socket_accept(client_listen);
	while(1) {
		printf("Accept connect from Master\n");
		if (client_process(master_request) != 0){
			break;
		}
	}
	close(master_request);
}
}