/*************************************************************************
	> File Name: test_client.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月17日 星期日 18时15分12秒
 ************************************************************************/

#include "common.h"


int main(){
    int len = 88;
    int fd[88];
    int port = 9999;

    char ip[20] = "192.168.2.101";
    for(int i = 0; i < len; i++){
        fd[i] = sock_connect(port, ip);
        if(fd[i] < 0){
            printf("error in fd[%d] connect!\n", i);
            continue;
        }
        close(fd[i]);
    }
    return 0;
}

