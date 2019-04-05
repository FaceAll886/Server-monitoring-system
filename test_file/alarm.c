/*************************************************************************
	> File Name: alarm.c
	> Author: 
	> Mail: 
	> Created Time: 2019年03月19日 星期二 20时43分05秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<sys/time.h>

void handle_signal(int sigNum){
    if(sigNum == SIGALRM){
        printf("hello world\n");
   //     alarm(10);
    }
}


int main(){
    signal(SIGALRM, handle_signal);
//    alarm(10);

    struct itimerval time;
    
    time.it_value.tv_sec = 0;
    time.it_value.tv_usec = 1;
    time.it_interval.tv_sec = 10;
    time.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &time, NULL);
    
    while(1);

    return 0;
}
