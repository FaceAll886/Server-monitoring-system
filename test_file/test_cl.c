/*************************************************************************
	> File Name: test_cl.c
	> Author: 
	> Mail: 
	> Created Time: 2019年04月04日 星期四 19时16分01秒
 ************************************************************************/

#include "../common/common.h"


int main(){
    int fd;
    fd = sock_connect(7731, "192.168.1.40");
    char buff[1024] = {0};

    while(scanf("%s", buff) != EOF){
        memset(buff, 0, sizeof(buff));
        send(fd, buff, strlen(buff) + 1, 0);
        memset(buff, 0, sizeof(buff));
        recv(fd, buff, sizeof(buff), 0);
        printf("%s\n", buff);
    }


    return 0;
}
