/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月16日 星期六 14时11分53秒
 ************************************************************************/

#include "../common/common.h"

struct sm_msg{
    int flag;                     //检测次数
    int sm_time;                  
    pthread_mutex_t sm_mutex;     //互斥锁
    pthread_cond_t sm_ready;      //条件变量
};

char *config = "../common/conf_log";
char *shared_memory = NULL;
char log_dir[50] = {0};
char log_backup[50] = {0};
double dynamic;     //memlog脚本的动态平均值

struct sm_msg *msg;
pthread_mutexattr_t m_attr;
pthread_condattr_t c_attr;

int client_heart(char *ip, int port){
    int fd;
    if((fd = sock_connect(port, ip)) < 0){
        return -1;
    }
    return fd;
}

int file_size(char *filename){
    struct stat statbuf;
    stat(filename, &statbuf);
    int size = statbuf.st_size;

    return size;
}

int compress_file(char *infilename, char *outfilename){
    int num_read = 0;
    FILE *infile = fopen(infilename, "rb");
    gzFile outfile = gzopen(outfilename, "wb");
    char inbuffer[128] = {0};
    unsigned long total_read = 0;

    while((num_read = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0){
        total_read += num_read;
        gzwrite(outfile, inbuffer, num_read);
        memset(inbuffer, 0, sizeof(inbuffer));
    }
    fclose(infile);
    gzclose(outfile);
    
    return 0;
}

int decompress_file(char *infilename, char *outfilename){
    int num_read = 0;
    char buffer[128] = {0};
    gzFile infile = gzopen(infilename, "rb");
    FILE *outfile = fopen(outfilename, "w");
    
    while((num_read = gzread(infile, buffer, sizeof(buffer))) > 0){
        fwrite(buffer, 1, num_read, outfile);
        memset(buffer, 0, sizeof(buffer));
    }
    
    gzclose(infile);
    fclose(outfile);
    return 0;
}

//检测是否有warning信息
int detect(char *buffer, int type){
    char *temp = strtok(buffer, "\n");
    char num[5] = {0};
    int j = 2;
    while(type){
        num[j--] = (type % 10) + '0';
        type /= 10;
    }
    num[3] = ' ';
    while(temp != NULL){
        char buff[20] = {0};
        strcat(buff, num);        //添加异常类型
        char *substr = NULL;
        char *substr2 = NULL; 
        substr = strstr(temp, "warning");
        substr2 = strstr(temp, "note");
        if(substr == NULL && substr2 == NULL){ 
            temp = strtok(NULL, "\n");
            continue;
        }
        if(substr != NULL) strcat(buff, "warning"); 
        else strcat(buff, "note");
        
        char port_u[10] = {0};
        char server_ip[20] = {0};
        get_conf_value(config, "Server_Ip", server_ip);
        get_conf_value(config, "Udp_Port", port_u);
        int port_udp = atoi(port_u);
        int sock_udp = socket(AF_INET, SOCK_DGRAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_udp);
        addr.sin_addr.s_addr = inet_addr(server_ip);
        
        //发送字符串使用strlen发送， 避免发送多余数据
        sendto(sock_udp, buff, strlen(buff) + 1, 0, (struct sockaddr *)&addr, sizeof(addr));  
        temp = strtok(NULL, "\n");
    }
    return 0;
}

void sys_detect(int type){
    int time_i = 0;
    char src[50] = {0};
    char run[50] = {0};
    sprintf(src, "Src%d", type);
    get_conf_value(config, src, src);
    FILE *fstream = NULL;
    char buffer[4096] = {0};
    char logname[50] = {0};
    int times = 0;
    char temp[4] = {0};
    char path[20] = {0};
    char inter[5] = {0};
    char fsize[15] = {0};
    char log_file[20] = {0};

    get_conf_value(config, "client_log", log_file);
    get_conf_value(config, "File_Max", fsize);
    get_conf_value(config, "Backfile", path);
    get_conf_value(config, "WriteEveryTime", temp);
    get_conf_value(config, "Interaction", inter);
    
    int fmsize = atoi(fsize);
    times = atoi(temp);
    int interaction = atoi(inter);
    switch(type) {
        case 100: {                   
            time_i = 5;
            sprintf(logname, "%s/cpu.log", path); 
            break; 
        }
        case 101: {
            time_i = 5; 
            sprintf(logname, "%s/mem.log", path); 
            break; 
        }
        case 102: { 
            time_i = 60;   
            sprintf(logname, "%s/disk.log", path);
            break;
        }
        case 103: {
            time_i = 30;   
            sprintf(logname, "%s/proc.log", path); 
            break;
        }
        case 104: {
            time_i = 60;   
            sprintf(logname, "%s/sys.log", path); 
            break;
        }
        case 105: {
            time_i = 60; 
            sprintf(logname, "%s/user.log", path); 
            break;
        }
    }

    sprintf(run, "bash ./%s", src);
    if(type == 101) sprintf(run, "bash ./%s %lf", src, dynamic);

    while(1){

        for(int i = 0; i < times; i++){
            char buff[400] = {0};
            if(NULL == (fstream = popen(run, "r"))){
                write_Pi_log(log_file, "popen \"%s\" failed!\n", run);
                exit(1);
            }
            if(type == 101) {
                if(fgets(buff, sizeof(buff), fstream)){
                    strcat(buffer, buff);
                    strcat(buffer, "\n");
                }
                if(fgets(buff, sizeof(buff), fstream)){
                    dynamic = atof(buff);
                }else {
                    dynamic = 0;
                }
            } else {
                while(fgets(buff, sizeof(buff), fstream)){
                    strcat(buffer, buff);
                    strcat(buffer, "\n");
                }
            }
            sleep(time_i);
            pclose(fstream);
            if(type == 100){
                printf("\033[31m*\033[0m");
                fflush(stdout);
                pthread_mutex_lock(&msg->sm_mutex);
                if(msg->flag++ >= interaction - 1){
                    if(msg->sm_time == 0){
                        printf("系统自检超过\033[31m%d\033[0m 次, master 无法连接\n", msg->flag);
                        pthread_cond_signal(&msg->sm_ready);
                        printf("发送信号 心跳开始启动 ❤ \n");
                    }
                    msg->flag = 0;
                }
                pthread_mutex_unlock(&msg->sm_mutex);
            }
        }
        detect(buffer, type);    
//        printf("%s", buffer);
        if(strlen(buffer) != 0) strcat(buffer, "\n");   
        FILE *fd = fopen(logname, "a+");
        if(NULL == fd){
            write_Pi_log(log_file, "open %s file failed!\n", logname);
            exit(1);
        }
        if(flock(fileno(fd), LOCK_EX) < 0){     //LOCK_EX 排它锁多用写操作  LOCK_SH共享锁多用读操作 LOCK_UN 释放
            write_Pi_log(log_file, "flock: %s\n", strerror(errno));
        }
        fwrite(buffer, 1, strlen(buffer), fd);
        fclose(fd);          //关闭文件指针也就讲锁释放了
        if(file_size(logname) > fmsize){      //文件过大进行压缩存储
           /* time_t timep = time(NULL);
            struct tm *t;
            t = localtime(&timep);*/
            char newfile[20] = {0};
            sprintf(newfile, "%s_gz", logname);
            compress_file(logname, newfile);
            remove(logname);             //备份文件后删除原文件
        }
    }
    return ;
}

int send_file(int sock_ctrl, int num1, int msg_port, int msg_fd){
    char logname[30] = {0};
    char path[10] = {0};
    char log_file[20] = {0};
    get_conf_value(config, "client_log", log_file);
    get_conf_value(config, "Backfile", path);

    switch(num1) {
        case 100: sprintf(logname, "%s/cpu.log", path);  break; 
        case 101: sprintf(logname, "%s/mem.log", path);  break;
        case 102: sprintf(logname, "%s/disk.log", path); break;
        case 103: sprintf(logname, "%s/proc.log", path); break;
        case 104: sprintf(logname, "%s/sys.log", path);  break;
        case 105: sprintf(logname, "%s/user.log", path); break;   
    }

    char temp_name[30] = {0};    //判断是否存在备份文件
    sprintf(temp_name, "%s_gz", logname);
    FILE *fp;
    if(access(temp_name, 0) == 0){
        char new_name[20] = {0};
        sprintf(new_name, "%s.log", temp_name);
        decompress_file(temp_name, new_name);
        fp = fopen(new_name, "r");
    } else {
        fp = fopen(logname, "r");
    }
   
    int sign = 0;
    if(fp == NULL){
        num1 += 300;
    } else {
        num1 += 100;
        sign = 1;
    }
/*    char ch = fgetc(fp);
    int sign = 0;
    if(ch == EOF){
        num1 += 300;
    } else {
        num1 += 100;
        sign = 1;
    }*/

    send(sock_ctrl, &num1, sizeof(num1), 0);
    if(sign == 1){
        int sock_msg;
        sock_msg = accept(msg_fd, NULL, NULL);
        if(sock_msg < 0){
            write_Pi_log(log_file, "msg_fd accept error! port is %d\n", msg_port);
            printf("sock_msg accept failed!\n");
            return -1;
        }
        char buffer[200] = {0};
        if(flock(fileno(fp), LOCK_SH) < 0){   //共享锁多个进程同用一把锁 可读文件
            write_Pi_log(log_file, "flock LOCK_SH failed: %s\n", strerror(errno));
        }
        while(fgets(buffer, sizeof(buffer), fp)){
            send(sock_msg, buffer, sizeof(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
        }
        fclose(fp);
        close(sock_msg);
//        remove(logname);                       //发送数据之后请空文件
    }                      
    return 0;
}

int main(){
    int shmid;
    int heart_listen;
    int port_heart, port_server;
    char temp_port[10] = {0};
    char temp_server[10] = {0};
    char ip_master[20] = {0};
    char temp_max[20] = {0};
    char ctrl_p[10] = {0};
    char msg_p[10] = {0};
    char log_file[20] = {0};

    get_conf_value(config, "client_log", log_file);
    get_conf_value(config, "Msg_Port", msg_p);
    get_conf_value(config, "Ctrl_Port", ctrl_p);
    get_conf_value(config, "Client_Port", temp_port);
    get_conf_value(config, "Server_Port", temp_server);
    get_conf_value(config, "Server_Ip", ip_master);
    get_conf_value(config, "MaxTimes", temp_max);
    
    int msg_port = atoi(msg_p);       //8732
    int ctrl_port = atoi(ctrl_p);     //9000
    int max_times = atoi(temp_max);
    port_heart = atoi(temp_port);     //10004
    port_server = atoi(temp_server);  //8731

    //建立共享内存区
    if((shmid = shmget(IPC_PRIVATE, sizeof(struct sm_msg), 0666|IPC_CREAT)) == -1){
        write_Pi_log(log_file, "shmget failed!\n");
        return -1;
    }
    shared_memory = (char *)shmat(shmid, 0, 0);
    if(shared_memory == NULL){
        write_Pi_log(log_file, "shmat failed!\n");
        return -1;
    }

    msg = (struct sm_msg *)shared_memory;
    msg->flag = 0;
    msg->sm_time = 0;
    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);
    pthread_mutexattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED); 
    pthread_condattr_setpshared(&c_attr, PTHREAD_PROCESS_SHARED);
    
    //初始化互斥锁和条件变量
    pthread_mutex_init(&msg->sm_mutex, &m_attr);
    pthread_cond_init(&msg->sm_ready, &c_attr);

    connect_nonblock(port_server, ip_master);   //上线就与master连接8731 并且断开
    
    int pid;

    if((pid = fork()) < 0){
        write_Pi_log(log_file, "fork first failed!\n");
        return -1;
    }

    if(pid != 0){
        printf("父进程\n");
        if((heart_listen = sock_create(port_heart)) < 0){  //10004
            write_Pi_log(log_file, "socket create heart_listen failed! port is %d\n", port_heart);
            return -1;
        }
        while(1){
            int fd;
            if((fd = accept(heart_listen, NULL, NULL)) < 0){
                write_Pi_log(log_file, "heart_listen accept:%s\n", strerror(errno));
                close(fd);
            }
            printf("\033[31m*\033[0m");   
            fflush(stdout);
               
            char temp[3] = "ok";
            if(recv(fd, temp, sizeof(temp), 0) == 0){
        
                printf("\033[31m@\033[0m");
                fflush(stdout);
                pthread_mutex_lock(&msg->sm_mutex);
                msg->flag = 0;
                msg->sm_time = 0;
                pthread_mutex_unlock(&msg->sm_mutex);
            }
        }
    } else {
        int pid_2;
        if((pid_2 = fork()) < 0){
            write_Pi_log(log_file, "fork second failed: %s\n", strerror(errno));
            return -1;
        }
        if(pid_2 == 0){   //避免网络信号差时服务端心跳未检测客户端在线，服务端讲客户端删除，客户端尝试心跳连接
            printf("子进程监听中...  %d\n", port_server);      //8731服务端是否在线
            while(1){
                pthread_mutex_lock(&msg->sm_mutex);
                printf("子进程等待信号开启心跳！\n");
                pthread_cond_wait(&msg->sm_ready, &msg->sm_mutex);
                printf("获得心跳信号，开始心跳  ❤ \n");
                pthread_mutex_unlock(&msg->sm_mutex);
                while(1){
                    int heart_fd = client_heart(ip_master, port_server);
                    if(heart_fd > 0){   //如果master断线，client尝试连接8731
                        printf("\n第%d次心跳成功!\n", msg->flag);
                        pthread_mutex_lock(&msg->sm_mutex);
                        msg->sm_time = 0;
                        msg->flag = 0;
                        pthread_mutex_unlock(&msg->sm_mutex);

                        //主动连接服务端， 发送提示信息
                        char name[30] = {0};
                        char send_buf[100] = {0};
                        gethostname(name, 30);
                        sprintf(send_buf, "Login->%s on server", name);
                        send(heart_fd, send_buf, strlen(send_buf) + 1, 0);
                        char buffer[100] = {0};
                        recv(heart_fd, buffer, sizeof(buffer), 0);
                        printf("%s\n", buffer);

                        close(heart_fd);
                        fflush(stdout);
                        break;
                    } else {
                        printf("\n第%d次心跳失败!\n", msg->sm_time);
                        pthread_mutex_lock(&msg->sm_mutex);
                        msg->sm_time++;
                        pthread_mutex_unlock(&msg->sm_mutex);
                        fflush(stdout);
                    }
                    if(msg->sm_time > max_times) msg->sm_time = max_times;
                    sleep(60 * msg->sm_time);
                    pthread_mutex_unlock(&msg->sm_mutex);
                }
            }   
        } else {
            printf("孙子进程\n");
            int pid_3;
            int x = 0;
            for(int i = 100; i <= 105; i++){    //开6个进程分别写.log文件
                x = i;
                if((pid_3 = fork()) < 0){
                    write_Pi_log(log_file, "fork third failed: %s\n", strerror(errno));
                    continue;
                }
                if(pid_3 == 0) break;    //子进程执行到这一步结束
            }
            if(pid_3 == 0){
                sys_detect(x);
            } else {
                printf("Father!\n");
                
                int client_listen = 0;
                if((client_listen = sock_create(ctrl_port)) < 0){   //9000 控制传输和数据传输
                    write_Pi_log(log_file, "client_listen failed: %s\n", strerror(errno));
                }
                while(1){
                    int sock_ctrl; 
                    if((sock_ctrl = accept(client_listen, NULL, NULL)) < 0){
                        write_Pi_log(log_file, "client_listen accept failed: %s!\n", strerror(errno));
                        continue;
                    }
                    printf("\033[31m#\033[0m");
                    fflush(stdout);
                    int msg_fd = 0;
                    if((msg_fd = sock_create(msg_port)) < 0){
                        write_Pi_log(log_file, "msg_fd socket create failed!\n");
                        perror("msg_fd create failed");
                        return -1;
                    }
                    /*char name[30] = {0};
                    char send_buf[100] = {0};
                    gethostname(name, 30);
                    sprintf(send_buf, "Login->%s on server", name);
                    send(heart_fd, send_buf, strlen(send_buf) + 1, 0);
                    char buffer[100] = {0};
                    recv(heart_fd, buffer, sizeof(buffer), 0);
                    printf("%s\n", buffer);*/

                    int num1;
                    while(recv(sock_ctrl, &num1, sizeof(num1), 0) > 0){
                        send_file(sock_ctrl, num1, msg_port, msg_fd);
                    }
                    printf("send finish!\n");
                    close(msg_fd);
                    close(sock_ctrl);
                    pthread_mutex_lock(&msg->sm_mutex);    //master请求数据说明已连接 停止心跳
                    msg->flag = 0;
                    msg->sm_time = 0;
                    pthread_mutex_unlock(&msg->sm_mutex);
                }
            }
        }
    }

    return 0;
}
