#ifndef CLIENT_H
#define CLIENT_H


#include "../common/common.h"

void send_file(int sockfd, char* filename);

int client_process(int master_request); 
//int client_recv_req(int master_request, char* req, char* arg);
int client_start_data_conn(int master_request);

int check_log_file(int reqcode, char *log);

#endif
