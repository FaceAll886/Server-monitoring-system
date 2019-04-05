#ifndef MASTER_H
#define MASTER_H


#include "../common/common.h"

int connet_client(char *host_t, char* port_t);

int read_reply(int master_request);

int master_get(int data_sock, int reqcode, char *client);

int master_create_conn(int sock_conn);

int master_send_req(int master_request, int req);

int request_file (int master_request, int reqcode, char *client);
int request_all(char *hostname, char *port);

#endif
