/*************************************************************************
	> File Name: test_mysql.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月24日 星期日 15时22分40秒
 ************************************************************************/

#include "common.h"

int main(){
    MYSQL *conn_ptr;
    conn_ptr = mysql_init(NULL);

    mysql_connect(conn_ptr);
    
    if(conn_ptr){
        printf("connect success!\n");
    } else {
        printf("connect failed!\n");
    }

    int ret;
    char time[20] = "2018-12-21 10:25:30";
    char ans[1000] = {0};
    sprintf(ans, "INSERT INTO warning_events(wtime, wip, wtype, wdetails) VALUES('%s', '192.168.2.101', 100, 'note')", time);
    
    printf("ans :%s\n", ans);
    ret = mysql_query(conn_ptr, ans);
    
    if(ret != 0){
        printf("insert success!\n");
    } else {
        printf("insert failed!\n");
    }

    mysql_close(conn_ptr);

    return 0;
}
