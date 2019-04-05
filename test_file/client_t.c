/*************************************************************************
	> File Name: client_t.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月23日 星期六 15时46分30秒
 ************************************************************************/

#include "common.h"


int main(){

    int sock_fd = sock_connect(8731, "192.168.2.101");

    if(sock_fd < 0){
        printf("error in sock_fd create\n");
        return -1;
    }

    while(1);

    close(sock_fd);


    return 0;
}
