/*************************************************************************
	> File Name: common.c
	> Author: 
	> Mail: 
	> Created Time: 2019年02月23日 星期六 11时00分29秒
 ************************************************************************/

 #include"common.h"

char config_2[20] = "../common/conf_log";

 int sock_create(int port){
    int server_listen;
    struct sockaddr_in server_addr;
    char log_file[20] = {0};
    get_conf_value(config_2, "master_log", log_file);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((server_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        write_Pi_log(log_file, "create socket:%s\n", strerror(errno));
        return -1;
    }

    int yes = 1;

    int ans = setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(ans < 0){  
        close(server_listen);
        write_Pi_log(log_file, "setsockopt:%s\n", strerror(errno));
        return -1;
    }


    if(bind(server_listen, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        write_Pi_log(log_file, "bind:%s\n", strerror(errno));
        close(server_listen);
        return -1;
    }
    if(listen(server_listen, 20) < 0){    //三次握手在listen之后accept之前完成
        write_Pi_log(log_file, "listen:%s\n", strerror(errno));
        close(server_listen);
        return -1;
    }

    return server_listen;
 }

int udp_create(int port){
    int sock_fd;
    struct sockaddr_in addr;
    char log_file[20] = {0};
    get_conf_value(config_2, "master_log", log_file);
    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        write_Pi_log(log_file, "udp_socket create:%s\n", strerror(errno));
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    
    int yes = 1;

    int ans = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(ans < 0){  
        close(sock_fd);
        write_Pi_log(log_file, "setsockopt:%s\n", strerror(errno));
        return -1;
    }

    if(bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        write_Pi_log(log_file, "bind:%s\n", strerror(errno));
        close(sock_fd);
        return -1;
    } 

    return sock_fd;
}
    
int sock_connect(int port, char *host){
    int sock_listen;
    struct sockaddr_in client_addr;
    char log_file[20] = {0};
    get_conf_value(config_2, "master_log", log_file);

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr.s_addr = inet_addr(host);
    
    if((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        write_Pi_log(log_file, "create socket:%s\n", strerror(errno));
        return -1;
    }
    //connect始终保持阻塞状态， 只有三次握手成功或者超时才返回
    if(connect(sock_listen, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0){  
        write_Pi_log(log_file, "connect:%s\n", strerror(errno));
        close(sock_listen);
        return -1;
    }

    return sock_listen;
}

int connect_nonblock(int port, char *host){ //客户端上线连接服务端 发送提示消息之后 立刻断开连接
    int sock_fd;
    struct sockaddr_in addr;
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0 )) < 0){
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    struct timeval tm;
    unsigned long ul = 1;
    ioctl(sock_fd, FIONBIO, &ul);
    fd_set set;
    int error = -1;
    int ret = -1;
    if(connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        tm.tv_sec = 0;
        tm.tv_usec = 300000;
        FD_ZERO(&set);
        FD_SET(sock_fd, &set);
        int len = sizeof(int);
        if(select(sock_fd + 1, NULL, &set, NULL, &tm) > 0) {
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
   
    if(ret > 0){
        ul = 0;
        ioctl(sock_fd, FIONBIO, &ul);
        char name[30] = {0};
        gethostname(name, 30);
        /*struct hostent *temp_host;
        temp_host = gethostbyaddr((char *)&addr.sin_addr.s_addr, 4, AF_INET);
        printf("temp_host name: %s\n", temp_host->h_name);*/
        
        char send_buf[100] = {0};
        sprintf(send_buf, "Login->%s on server", name);
        send(sock_fd, send_buf, strlen(send_buf) + 1, 0);
        
        char buffer[200] = {0};
        recv(sock_fd, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
    } else {
        printf("connect nonblock failed!\n");
    }
    close(sock_fd);
    return ret;
}


int get_conf_value(const char *pathname, const char *key_name, char *value){
    FILE *fd = NULL;
    char *line = NULL;
    char *substr = NULL;
    ssize_t read = 0;
    size_t len = 0;

    fd = fopen(pathname, "r");

    if(fd == NULL){
        printf("error in get_conf_value\n");
        exit(1);
    }
    
    while((read = getline(&line, &len, fd)) != -1){   //getline()返回字节数
        substr = strstr(line, key_name);   //key_name是否为line的子串
        if(substr == NULL){
            continue;
        } else {
            int temp = strlen(key_name);
            if(line[temp] == '='){
                strncpy(value, &line[temp + 1], (int)read - temp - 1);
                temp = strlen(value);
                *(value + temp - 1) = '\0';
                break;
            }
            else{
                printf("Next\n");
                continue;
            }
        }
    }

/*    while((read = getline(&line, &len, fd)) != 1 ){
        printf("%s", line);
        char *temp = strtok(line, "=");
        if(strcmp(key_name,temp) == 0){
            strncpy(value, &line[strlen(key_name) + 1], (int)read - strlen(key_name) - 2);
          *(value + strlen(key_name) - 1) = '\0';
            break;
        } else {
            continue;
        } 
    }*/
    return 0;
}

int write_Pi_log (char *PiHealthLog, const char *format, ...){
    time_t timep = time(NULL);
    struct tm *t;
    FILE *fp;
    int ret;
    va_list arg;
    va_start(arg, format);

    t = localtime(&timep);
    fp = fopen(PiHealthLog, "a+");

    if(fp == NULL){
        printf("error in write_Pi_log!\n");
        exit(1);
    }
    
    int a = t->tm_year + 1900;
    int b = t->tm_mon + 1;
    int c = t->tm_mday;
    int d = t->tm_hour;
    int e = t->tm_min;
    int f = t->tm_sec;    

    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d ", a, b, c, d, e, f);
    ret = vfprintf(fp, format, arg);

    fflush(fp);
    fclose(fp);
    va_end(arg);

    return ret;
}
 
int mysql_connect(MYSQL *conn_ptr){
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "rst138.", "pihealth", 0, NULL, 0);
    
    if(!conn_ptr){
        perror("connect mysql:\n");
        return -1;
    }

    return 0;
}

int get_time(char *cur_time){
    time_t timep = time(NULL);
    struct tm *t;
    t = localtime(&timep);
    
    int a = t->tm_year + 1900;
    int b = t->tm_mon + 1;
    int c = t->tm_mday;
    int d = t->tm_hour;
    int e = t->tm_min;
    int f = t->tm_sec;    

    sprintf(cur_time, "%04d-%02d-%02d %02d:%02d:%02d", a, b, c, d, e, f);

    return 0;
}
