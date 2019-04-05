#include "master.h"

int master_request;
char data_dir[100] = {0};

int connet_client(char *host_t, char* port_t) {
	struct addrinfo hints, *res, *rp;
	printf("Try to connect with client %s \n", host_t);
	memset(&hints, 0, sizeof(struct addrinfo));
	char buffer[MAX_SIZE];

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int s = getaddrinfo(host_t, port_t, &hints, &res);

	if (s != 0 ) {
		printf("getaddrinfo() error %s\n", gai_strerror(s));

	} 

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		master_request = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if(master_request < 0)
			continue;

		if (connect(master_request, res->ai_addr, res->ai_addrlen) == 0) {
			recv(master_request, buffer, MAX_SIZE, 0);
			printf("%s\n", buffer);
			break;
	
		} else {
			perror("connecting stream socket error ");
			printf("%s\n", host_t);
			exit(1);
		}
		close(master_request);
	}
	freeaddrinfo(rp);
	printf("Connection to %s is OK\n", host_t);
	return master_request;	
}

int read_reply(int master_request) {
	int retcode = 0;

	if (recv(master_request, &retcode, sizeof(retcode), 0) < 0) {
		perror("master : error reading message from client");
		return -1;
	}
	//return ntohl(retcode);
	return retcode;
}


int master_get(int data_sock, int reqcode, char *client) {
	char data[MAX_SIZE];
	int size;
	char arg[MAX_SIZE] = {0};
	strcpy(arg, data_dir);
	strcat(arg,"/");
	strcat(arg, client);

	switch (reqcode) {
		case 100:
			strcat(arg, "/cpu.log");
			break;
		case 101:
			strcat(arg, "/mem.log");
			break;
		case 102:
			strcat(arg, "/disk.log");
			break;
		case 103:
			strcat(arg, "/proc.log");
			break;
		case 104:
			strcat(arg, "/sysinfo.log");
			break;
		case 105:
			strcat(arg, "/users.log");
			break;
		default:
			break;
	}

	FILE* fd = fopen(arg, "a+");

	while ((size = recv(data_sock, data, 100, 0)) > 0) {
		fwrite(data, 1, size, fd);
	}

	if (size < 0) 
		perror("error\n");

	fclose(fd);
	printf("Write massage to File %s\n", arg);
	return 0;
}

int master_create_conn(int sock_conn) {
	
	int master_listen = socket_create(MASTER_PORT);

	int ack = 1;

	if ((send(sock_conn, (char*) &ack, sizeof(ack), 0) < 0)) {
		printf("master : ack writer error %d",errno);
		exit(1);
	}
	printf("Send ACK to client in order to file transfer\n");
	int sock_conn_con = socket_accept(master_listen);
	close(master_listen);
	return sock_conn_con;
}


int master_send_req(int master_request, int req) {

	//int req_send = htonl(req);
	int req_send = req;
	if (send(master_request, &req_send, sizeof(req_send), 0) <0 ) {
		perror("error sending req");
		return -1;
	}

	return 0;
}

int request_file (int master_request, int reqcode, char *client) {
	int retcode, data_sock;
	printf("Send REQUEST code %d to %s for log file \n",reqcode, client);
	if (master_send_req(master_request, reqcode) < 0) {
		perror("master_send_req() error");
		exit(1);
	}

	retcode = read_reply(master_request);
	printf("recive code from %s retcode = %d\n", client, retcode);
	
	if (retcode == reqcode + 100)
	{
		printf("Master create the Socket for file transfer\n");
		if ((data_sock = master_create_conn(master_request)) < 0) {
			perror("Error opening socket for data connn\n");
			exit(1);
		}
		printf("Connection OK, Start  file transfer\n");
		master_get(data_sock, reqcode, client);
		printf("Mater stop the Connection\n")  ;
		close(data_sock);
	} else if ( retcode == reqcode + 300) {
		printf("There is no file on %s for REQUEST %d\n", client, retcode);
		return -1;
	} else if (retcode == reqcode + 400) {
		printf("All file has finished for REQUEST %d\n", reqcode);
		return -1;
	} else if (retcode == reqcode + 200) {
		printf("Nothing can happen here\n");
	} else {
		printf("worry retcode FOR REQUEST %d \n",reqcode);
		return -1;
	}

	return 0;
}

int request_all(char *hostname, char *port) {
	int master_request = connet_client(hostname, port);
	for (int i = 100; i < 106; i++)
	{
		request_file(master_request, i, hostname);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	//int data_sock, retcode;
	char port_t[5] = {0}; //if port = 5 ,has error
	char host_t[10] = {0};
	char prename[10] = {0};
	char debugmode[2] = {0}; 
	int status;
	int x = 0;
	char *config = "/etc/pihealth.conf";
	if (argc != 1)
	{
		printf("Usage: ./master \n");
		exit(0);
	}



	get_conf_value(config, "client_port", port_t);
	get_conf_value(config, "client", host_t);
	get_conf_value(config, "datadir",data_dir);
	get_conf_value(config, "debug", debugmode);
	
	int tmp = atoi(debugmode);

	if (tmp == 1)
	{	
		request_all(host_t, port_t);

	} else if (tmp == 0) {
		char start[10] = {0};
		char finish[10] = {0};
		char hostname[20] = {0};
		get_conf_value(config, "prename", prename);
		get_conf_value(config, "start", start);
		get_conf_value(config, "finish", finish);
		int s = atoi(start);
		int f = atoi(finish);

		signal(SIGCHLD, SIG_IGN);
		for (int i = s; i <= f; i++)
		{
			status = fork();
			x = i;
			if(status == 0 || status == -1)
				break;
		}
		if (status == -1)
			printf("error\n");
		else if (status == 0) {
			sprintf(hostname, "%s%d", prename, x + 1);
			printf("In this child , we do with %s\n", hostname);
			request_all(hostname, port_t);
		} else {
			wait(NULL);
			printf("parents : all children finished!\n");
		}

	}

	return 0;
}


