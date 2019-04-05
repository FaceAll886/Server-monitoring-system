/*************************************************************************
	> File Name: udp_client.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月19日 星期二 20时12分43秒
 ************************************************************************/

#include"common.h"

int main(){
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
 
    char temp_port[5] = {0};
    char config[10] = "./conf_log";
    get_conf_value(config, "Udp_Port", temp_port);
    int port = atoi(temp_port);
    printf("%d\n", port);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("192.168.2.101");
    
    while(1){
        char buffer[256] = {0};
        scanf("%[^\n]", buffer);
        sendto(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, sizeof(addr));

    }
    
    close(sock_fd);
    return 0;
} 
