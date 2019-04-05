/*************************************************************************
	> File Name: common.h
	> Author: 
	> Mail: 
	> Created Time: 2019年02月23日 星期六 10时59分40秒
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>    
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mysql.h>
#include <zlib.h>
#include <io.h>
#include <sys/epoll.h>

struct Message{
    char from[20];
    int flag;
    char message[1024];
};

int sock_create(int port);

int sock_connect(int port, char *host); 

int get_conf_value(const char *pathname, const char *key_name, char *value);

int write_Pi_log(char *PiHealthLog, const char *format, ...);

int connect_nonblock(int port, char *host);

int udp_create(int port);

int mysql_connect(MYSQL *conn_ptr);

int get_time(char *time);

#endif
