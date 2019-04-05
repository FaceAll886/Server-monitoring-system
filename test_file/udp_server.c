/*************************************************************************
	> File Name: udp_server.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月19日 星期二 19时57分18秒
 ************************************************************************/

#include "common.h"

int main(){
    char config[20] = "./conf_log";
    char temp_port[5] = {0};

    get_conf_value(config, "Udp_Port", temp_port);

    int port = atoi(temp_port);
    int sock_fd;
    printf("%d\n", port);
    if((sock_fd = udp_create(port)) < 0){
        perror("sock create:");
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

//    while(1){
        char buffer[200] = {0};
        recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &len);
        printf("%s\n", buffer);
//    }
    close(sock_fd);
    return 0;
}
