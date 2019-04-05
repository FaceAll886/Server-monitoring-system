/*************************************************************************
	> File Name: test_epoll.c
	> Author: 
	> Mail: 
	> Created Time: 2019年04月04日 星期四 19时16分59秒
 ************************************************************************/

#include "../../chatroom/common.h"


int main(){
    int server_listen;
    if((server_listen = sock_create(7731)) < 0){
        perror("sock_create:");
        return -1;
    }

    int epoll_fd;
    epoll_fd = epoll_create(500);

    struct epoll_event ev, event[100];
    ev.events = EPOLLIN;
    ev.data.fd = server_listen;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_listen, &ev);

    while(1) {
        int nfds = epoll_wait(epoll_fd, event, 100, -1);
        for(int i = 0; i < nfds; i++){
            if(event[i].data.fd == server_listen && event[i].events &EPOLLIN){
                int sock_fd = accept(server_listen, NULL, NULL);
                if(sock_fd == -1){
                    perror("accept:");
                    continue;
                }
                
                //已连接的套接字即可读
                ev.data.fd = sock_fd;      
                ev.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev);
            } else if(event[i].events & EPOLLIN){
                char buffer[1024] = {0};
                int recvn = recv(sock_fd , buffer, sizeof(buffer), 0);
                if(recvn == 0){
                    ev.data.fd = event[i].data.fd;
                    close(ev.data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event[i].data.fd, 0);
                }else {
                    printf("%s\n", buffer);
                    fflush(stdout);

                    //已接受消息的套接字可写
                    ev.events = EPOLLOUT;
                    ev.data.fd = event[i].data.fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event[i].data.fd, &ev);   
                }
            } else if(event[i].events & EPOLLOUT){
                send(event[i].data.fd, "Hello!", 7, 0);
                ev.data.fd = event[i].data.fd;
                ev.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event[i].data.fd, &ev);
            }
        }
    }

    return 0;
}
