#include "common.h"



int get_conf_value(char* pathname, char* key_name, char *value) {

	FILE *fp = NULL;
	char *line = NULL, *substr = NULL;
	size_t len = 0, tmplen = 0;
	ssize_t read;
	//memset(value, 0, sizeof(char)*MAX_SIZE);

	if ( key_name == NULL || value == NULL) {
		printf("paramer is invaild!\n");
		exit(-1);
	}

	fp = fopen(pathname,"r");
	if (fp == NULL) {
		printf("Open config file error!\n");
		exit(-1);
	}

	while (( read = getline(&line, &len,fp)) != -1) {
		substr = strstr(line, key_name);
		if (substr == NULL) 
			continue;
		else {
			tmplen = strlen(key_name);
			if (line[tmplen] == '=') {
				strncpy(value, &line[tmplen + 1], (int)read - tmplen + 1);
				tmplen = strlen(value);
				*(value + tmplen - 1) = '\0';
				break;
			}
			else {
				printf("Maybe there is something wrong with config file\n");
				continue;
			}
		}
	}

	if (substr == NULL) {
		printf("%s not found in config file\n", key_name);
		fclose(fp);
		exit(-1);
	}

	printf("%s=%s\n", key_name, value);
	free(line);
	fclose(fp);
	return 0;
}



/*
*创建监听套接字
*返回套接字描述符，错误返回-1
*/

int socket_create(int port){
	int sockfd;
	int yes = 1;
	struct sockaddr_in sock_addr;

	//创建套接字
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error\n");
		return -1;
	}

	//设置本地套接字地址
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port); //转化为网络字节序
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0

	//设置本地套接字
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		perror("setsockopt() error\n");
		return -1;
	}

	//绑定本地套接字到套接字
	if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		close(sockfd);
		perror("bind() error\n");
		return -1;
	}

	//将套接字设置为监听状态
	if (listen(sockfd, 20) < 0) {
		close(sockfd);
		perror("listen() error");
		return -1;
	}
	return sockfd;
}


/*
*接受套接字请求
*返回新的套接字描述符，错误返回-1
*/
int socket_accept(int sock_listen) {
	int sockfd;
	struct sockaddr_in client_addr, server_addr;
	char buffer[MAX_SIZE];
	socklen_t len = sizeof(client_addr); 
	sockfd = accept(sock_listen, (struct sockaddr *) &client_addr, &len);
	getsockname(sockfd, (struct sockaddr *) &server_addr, &len);
	if (sockfd < 0){
		perror("accept() error");
		return -1;
	}
	sprintf(buffer, "%s:%d --> You have connected to Server!", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
	send(sockfd, buffer, strlen(buffer), 0);
	printf("%s:%d Login Server!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	return sockfd;
}


/*
*接受套接字请求
*返回新的套接字描述符，错误返回-1
*/

int socket_connect(int port, char *host) {
	int sockfd;
	struct sockaddr_in dest_addr;
	char buffer[MAX_SIZE];
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return -1;
	}

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(host);

	printf("Connetion TO %s:%d\n",host,port);
	fflush(stdout);
	if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		perror("connect() error");
		return -1;
	}
	recv(sockfd, buffer, MAX_SIZE, 0);
	printf("%s\n", buffer);
	return sockfd;

}

int recv_data(int sockfd, char* buf, int bufsize) {
	size_t num_bytes;
	memset(buf, 0, bufsize);

	num_bytes = recv(sockfd, &buf, bufsize, 0);

	if (num_bytes < 0) {
		return -1;
	}
	return num_bytes;
}



int send_response(int sockfd, int rq) {
	//int rq_send = htonl(rq);
	//if (send(sockfd, &rq_send, sizeof(rq_send), 0) <0 ) {
	if (send(sockfd, &rq, sizeof(rq), 0) <= 0 ) {
		perror("error sending rq");
		return -1;
	}
	return 0;
}


int recv_response(int sockfd) {
	int res_recv;
	if ((recv(sockfd, &res_recv, sizeof(int), 0)) <= 0) {
		perror("recv response error:");
		return -1;
	}
	return res_recv;
}

