/*************************************************************************
	> File Name: master.h
	> Author: 
	> Mail: 
	> Created Time: 2019年03月09日 星期六 18时09分17秒
 ************************************************************************/

#ifndef _MASTER_H
#define _MASTER_H

#include "../common/common.h"

typedef struct Node{
    struct sockaddr_in client_addr;
    struct Node *next;
}Node, *LinkedList;

struct Sock{
    int fd;
    struct sockaddr_in addr;
    int num;
};

struct Print{
    LinkedList head;
    int index;
};

struct Heart{
    LinkedList *head;
    int ins;
    int *sum;
};

pthread_rwlock_t lock_t;

int transIp(char *sip, int *ip){
    if(sip == NULL) return -1;
    char temp[4];
    int count = 0;
    while(1){
        int index = 0;
        while(*sip != '\0' && *sip != '.' && count < 4){
            temp[index++] = *sip;
            sip++;
        }
        if(index == 4) return -1;
        temp[index] = '\0';
        ip[count] = atoi(temp);
        count++;
        if(*sip == '\0'){
            if(count == 4) return -1;
        } else {
            sip++;
        }
    }
    return 1;
}

int insert(LinkedList head, Node *node){
    Node *p = head;
    while(p->next != NULL){
        p = p->next;
    }
    p->next = node;
    return 0;
}

int find_min(int *sum, int ins){
    int ret = 0;
    for(int i = 0; i < ins; i++){
        if(*(sum + i) < *(sum + ret))  ret = i;
    }
    return ret;
}

void *print(void *arg) {
    struct Print *print_para = (struct Print *)arg;
    int ind = print_para->index;
    char filename[50] = {0};
    sprintf(filename, "../link_log/%d.log", ind);
    while (1) {
        FILE *file = fopen(filename, "w");
        pthread_rwlock_rdlock(&lock_t);
        Node *p = print_para->head;
        pthread_rwlock_unlock(&lock_t);
        while(1){
            pthread_rwlock_rdlock(&lock_t);
            if(p->next == NULL){
                pthread_rwlock_unlock(&lock_t);
                break;
            }
            fprintf(file, "%s: %d\n", inet_ntoa(p->next->client_addr.sin_addr), ntohs(p->next->client_addr.sin_port));
            p = p->next;
            pthread_rwlock_unlock(&lock_t);
        }
        fflush(file);
        fclose(file);
        sleep(1);
    }
    return NULL;
}

/*int connect_sock_2(struct sockaddr_in addr){
    int sock_fd;
    
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    struct timeval tm;
    unsigned long ul = 1;
    ioctl(sock_fd, FIONBIO, &ul);
    int error = -1;
    int ret = -1;
    fd_set set;

    if(connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        tm.tv_sec = 0;
        tm.tv_usec = 300000;
        FD_ZERO(&set);
        FD_SET(sock_fd, &set);
        int len = sizeof(int);
        if(select(sock_fd + 1, NULL, &set, NULL, &tm) > 0){
            getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if(error == 0){
                ret = 1;
            } else {
                ret = -1;
            }
        } else {
            ret = -1;
        }
    } else {
        ret = 1;
    }
    
    close(sock_fd);
    return ret;
}*/

int connect_sock(struct sockaddr_in addr, fd_set *set, struct Sock socks[], int *j, int *maxfd, int i){
    int sock_fd;
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }
    
    unsigned long ul = 1;
    ioctl(sock_fd, FIONBIO, &ul);


    socks[*j].fd = sock_fd;
    socks[*j].addr = addr;
    socks[*j].num = i;

    *j = *j + 1;
    for(int i = 0; i < *j; i++){
        if(socks[i].fd > *maxfd) *maxfd = socks[i].fd;
    }
    
    if(connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        return -1;
    }
    
    return 0;
}

int check(LinkedList *head, int ins, struct sockaddr_in addr){
    for(int i = 0; i < ins; i++){
        Node *p = head[i];
        while(p->next != NULL){
            if(p->next->client_addr.sin_addr.s_addr == addr.sin_addr.s_addr) {
                return -1;
            }
            p = p->next;
        }
    }
    return 0;
}

void deletes(struct sockaddr_in addr, int i, LinkedList *lists){
    Node *p = lists[i];
    Node *q = p->next;
    char temp[20] = {0};
    strcpy(temp, inet_ntoa(addr.sin_addr));
    while(q != NULL){
        char ip[20] = {0};
        strcpy(ip, inet_ntoa(q->client_addr.sin_addr));
        if(strcmp(ip, temp) == 0){
            p->next = q->next;
            free(q);
            return ;
        }
        p = p->next;
        q = q->next; 
    }
    return ;
}

void *heartbeat(void *arg){
    struct Heart *heart;
    heart = (struct Heart *)arg;
   
    while(1){
        printf("hearting**************\n");
        struct Sock socks[1025];
        int j = 0;
        int maxfd;
        fd_set set;
        FD_ZERO(&set);
        for(int i = 0; i < heart->ins; i++){
            pthread_rwlock_rdlock(&lock_t);
            Node *p = heart->head[i];
            pthread_rwlock_unlock(&lock_t);
            while(1){
                pthread_rwlock_rdlock(&lock_t);
                if(!(p != NULL && p->next != NULL)){
                    pthread_rwlock_unlock(&lock_t);
                    break;
                }
                Node *temp_node = p->next;
                p = p->next;
                pthread_rwlock_unlock(&lock_t);
                connect_sock(temp_node->client_addr, &set, socks, &j, &maxfd, i);
                FD_SET(socks[j - 1].fd, &set);
            }       
        }
        struct timeval tm;
        tm.tv_sec = 1;
        tm.tv_usec = 0;
        sleep(1);   //睡眠1秒，让所有描述字均判断处于就绪可写态
        int rc = select(maxfd + 1, NULL, &set, NULL, &tm);   //set集合只要有一个描述字满足可写select即返回
        for(int i = 0; i < j; i++){
            if(rc > 0 && FD_ISSET(socks[i].fd, &set)){
                int error = -1;
                int len = sizeof(int);
                getsockopt(socks[i].fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                if(error == 0){ 
                    printf("\033[32m%s: %d online\033[0m\n", inet_ntoa(socks[i].addr.sin_addr), ntohs(socks[i].addr.sin_port));
                    rc--;
                } else {
                    printf("ip %s is deleting\n", inet_ntoa(socks[i].addr.sin_addr));
                    pthread_rwlock_wrlock(&lock_t);
                    deletes(socks[i].addr, socks[i].num, heart->head);
                    heart->sum[socks[i].num]--;
                    pthread_rwlock_unlock(&lock_t);
                }
            } else {
                printf("ip %s is deleting\n", inet_ntoa(socks[i].addr.sin_addr));
                pthread_rwlock_wrlock(&lock_t);
                deletes(socks[i].addr, socks[i].num, heart->head);
                heart->sum[socks[i].num]--;
                pthread_rwlock_unlock(&lock_t);
            }
            close(socks[i].fd);
        }
        sleep(5);
    }
    return NULL;
}

void find_file(int i, char *files, char *ip){
    switch(i){
        case 100 : sprintf(files, "../files/%s/cpu.log", ip); break;
        case 101 : sprintf(files, "../files/%s/mem.log", ip); break;
        case 102 : sprintf(files, "../files/%s/disk.log", ip); break;
        case 103 : sprintf(files, "../files/%s/pro.log", ip); break;
        case 104 : sprintf(files, "../files/%s/sys.log", ip); break;
        case 105 : sprintf(files, "../files/%s/user.log", ip); break;
    }
    return ;
}

void *recv_file(void *arg){
    struct Heart *file;
    file = (struct Heart *)arg;
    
    char temp_ctrl[10] = {0};
    char temp_msg[10] = {0};
    char server_ip[20] = {0};
    char config[20] = "../common/conf_log";
    char log_file[20] = {0};

    get_conf_value(config, "master_log", log_file);
    get_conf_value(config, "Server_Ip", server_ip);
    get_conf_value(config, "Ctrl_Port", temp_ctrl);
    get_conf_value(config, "Msg_Port", temp_msg);
    int msg_port = atoi(temp_msg);
    int ctrl_port = atoi(temp_ctrl);

    while(1){
        sleep(10);
        printf("traversing list---------\n");
        for(int i = 0; i < file->ins; i++) {

            pthread_rwlock_rdlock(&lock_t);
            Node *p = file->head[i];
            pthread_rwlock_unlock(&lock_t);

            while(1){

                pthread_rwlock_rdlock(&lock_t);
                if(!(p != NULL && p->next != NULL)){
                    pthread_rwlock_unlock(&lock_t);
                    break;
                }
                Node *temp_node = p->next;
                p = p->next;
                pthread_rwlock_unlock(&lock_t);

                char ip[20] = {0};
                strcpy(ip, inet_ntoa(temp_node->client_addr.sin_addr));
                int sock_ctrl;
                if((sock_ctrl = sock_connect(ctrl_port, ip)) < 0){
                    write_Pi_log(log_file, "sock_ctrl connect failed! ip is %s port is %d\n", ip, ctrl_port);
                    continue;
                }
                char ff[30] = {0};
                sprintf(ff, "../files/%s", ip);
                mkdir(ff, 0755);   

                for(int i = 100; i <= 105; i++){
                    send(sock_ctrl, &i, sizeof(i), 0);    
                    int temp = 0;
                    recv(sock_ctrl, &temp, sizeof(temp), 0);
                    if(temp == i + 100){
                        int msg_fd;
                        if((msg_fd = sock_connect(msg_port, ip)) < 0){
                            write_Pi_log(log_file, "msg_fd connect failed!ip is %s port is %d\n", server_ip, msg_port);
                            continue;
                        }
                        char files[30] = {0};
                        find_file(i, files, ip);
                        FILE *fp = fopen(files, "w");
                        char buffer[200] = {0};
                        while(recv(msg_fd, buffer, sizeof(buffer), 0) > 0){
                            fprintf(fp, "%s", buffer);
                            memset(buffer, 0, sizeof(buffer));
                        }
                        fclose(fp);    
                        close(msg_fd);
                    } else {
                        write_Pi_log(log_file, "recv file failed! file type is %d\n", i);
                        continue;
                    }
                }
                printf("recv success!\n");
                close(sock_ctrl);
            }
        }
    }
}

void *udp_buff(void *arg){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int *p = (int *)arg;
    int sock_fd = *p;
    MYSQL *conn_ptr;
    conn_ptr = mysql_init(NULL);
    char log_file[20] = {0};
    char config[20] = "../common/conf_log";
    get_conf_value(config, "master_log", log_file);

    mysql_connect(conn_ptr);

    while(1){
        char buffer[20] = {0};
        recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &len);
        char ip[20] = {0};
        strcpy(ip, inet_ntoa(addr.sin_addr));
        char cur_time[20] = {0};
        get_time(cur_time);
        int type = 0;
        char details[15] = {0};
        int j = 0;
        for(int i = 0; buffer[i]; i++){
            if(buffer[i] >= '0' && buffer[i] <= '9'){
                type = type * 10 + (buffer[i] - '0');
                continue;
            } 
            if(buffer[i] != ' '){
                details[j++] = buffer[i];
            }
        }
        details[j] = '\0';
        char sql_state[100] = {0};
        if(strcmp(details, "warning") == 0) {
            sprintf(sql_state, "INSERT INTO warning_events(wtime, wip, wtype, wdetails) VALUES('%s','%s',%d,'%s')", cur_time, ip, type, details);
        } else {
            sprintf(sql_state, "INSERT INTO note_events(wtime, wip, wtype, wdetails) VALUES('%s','%s',%d,'%s')", cur_time, ip, type, details);
        }
            if(mysql_query(conn_ptr, sql_state) != 0){
                write_Pi_log(log_file, "insert mysql failed\n");
            continue;
        }
        printf("insert mysql %s_events success!\n", details);
    } 
}

int do_epoll(int server_listen, int *sum, int ins, LinkedList *list){
    unsigned long ul = 1;
    ioctl(server_listen, FIONBIO, &ul);
    char config[20] = "../common/conf_log";
    char log_file[20] = {0};
    char temp_port[10] = {0};
    get_conf_value(config, "Client_Port", temp_port);
    get_conf_value(config, "master_log", log_file);
    int port = atoi(temp_port);
    
    int epoll_fd = epoll_create(5000);      //大小无实际意义
    if(epoll_fd < 0){
        write_Pi_log(log_file, "epoll_fd create:%s\n", strerror(errno));
    }
    struct epoll_event ev, events[1000];
    ev.data.fd = server_listen;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_listen, &ev);
    
    while(1){
        int fds = epoll_wait(epoll_fd, events, 1000, -1);
        
        if(fds < 0){
            write_Pi_log(log_file, "epoll_wait:%s\n", strerror(errno));
            break;
        }

        for(int i = 0; i < fds; i++){
            if(events[i].data.fd == server_listen){   //有新的连接服务端监听套接字, accept
                struct sockaddr_in addr;
                socklen_t len = sizeof(addr);
                int connfd = accept(server_listen, (struct sockaddr *)&addr, (socklen_t *)&len);
                if(connfd > 0){
                    
                    //添加到链表
                    int sub = find_min(sum, ins);
                    Node *node = (Node *)malloc(sizeof(Node));
                    node->client_addr = addr;
                    node->client_addr.sin_port = htons(port);
                    node->next = NULL;
                    pthread_rwlock_rdlock(&lock_t);
                    int ret = check(list, ins, addr);
                    pthread_rwlock_unlock(&lock_t);
                    if(ret == -1){
                        write_Pi_log(log_file, "ip is %s exist\n", inet_ntoa(addr.sin_addr));
                        continue;
                    }
                    pthread_rwlock_wrlock(&lock_t);
                    insert(list[sub], node);
                    pthread_rwlock_unlock(&lock_t);
                    sum[sub]++;

                } else {
                    write_Pi_log(log_file, "server_listen accept:%s\n", strerror(errno));
                }

                ev.data.fd = connfd;
                ev.events = EPOLLIN | EPOLLET;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    write_Pi_log(log_file, "epoll_ctl: %s\n", strerror(errno));
                }

            } else if(events[i].events & EPOLLIN){    //接收到客户端发来的数据， 读sock
                int sock_fd = events[i].data.fd;
                if(sock_fd < 0){
                    write_Pi_log(log_file, "EPOLLIN socket fd error!\n");
                    continue;
                }
                
                char buffer[200] = {0};
                recv(sock_fd, buffer, sizeof(buffer), 0);

                ev.data.fd = sock_fd;
                ev.events = EPOLLOUT | EPOLLET;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &ev) == -1) { //修改事件标识符
                    write_Pi_log(log_file, "epoll_ctl: %s\n", strerror(errno));
                }    
            } else if(events[i].events & EPOLLOUT){  //有数据等待发送, 写socket
                int sock_fd = events[i].data.fd;
                if(sock_fd < 0){
                    write_Pi_log(log_file, "EPOLLIN socket fd error!\n");
                    continue;
                }
                
                char send_buf[200] = {0};
                char name[30] = {0};
                gethostname(name, 30);
                sprintf(send_buf, "You Have Login on %s!\n", name);
                send(sock_fd, send_buf, strlen(send_buf) + 1, 0);
                
                ev.data.fd = sock_fd;
                ev.events = EPOLLIN | EPOLLET;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &ev) == -1) { //修改事件标识符
                    write_Pi_log(log_file, "epoll_ctl: %s\n", strerror(errno));
                }
            }
        }
    }
    close(epoll_fd);    //一定关闭文件描述字epoll_fd
    return 0;
}

#endif
