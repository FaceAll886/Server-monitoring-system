/*************************************************************************
	> File Name: master_t.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月23日 星期六 14时55分04秒
 ************************************************************************/

#include "common.h"


int main(){
    int port = 8731;
    int sock_fd = sock_create(port);

    if(sock_fd < 0){
        printf("error in create\n");
        return -1;
    }


    struct sockaddr_in caddr;
    int len = sizeof(caddr);

    while(1){
        int server_listen;
        if((server_listen = accept(sock_fd,(struct sockaddr *)&caddr, (socklen_t *)&len)) < 0){
            printf("error in accept\n");
            continue;
        }

        printf("%s    %d\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
    }

    return 0;
}
