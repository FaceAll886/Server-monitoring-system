/*************************************************************************
	> File Name: test_server.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月17日 星期日 18时15分31秒
 ************************************************************************/
#include "common.h"

int main(){
    int port = 9999;
    int sock_fd = sock_create(port);
    
    if(sock_fd < 0){
        perror("sock_fd create:");
        return -1;
    }

    while(1){
        int fd;
        if((fd = accept(sock_fd, NULL, NULL)) < 0){
            printf("error in fd accept\n");
            continue;
        }
    }
    close(sock_fd);
    return 0;
}
