#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


int x = 0;
int status;
int main() {
	signal(SIGCHLD, SIG_IGN);
	for (int i = 2; i <= 10; i++) {

		status = fork();
		x = i;
		if(status == 0 || status == -1)
			break;
		
	}
	if (status == -1)
		printf("error\n");
	else if (status == 0) {
		printf("the child %d  pid = %d  ppid=%d\n", x, getpid(), getppid());
	} else {
		wait(NULL);
		printf("parents : all children finished!\n");
		}
}