/*************************************************************************
	> File Name: test_file.cpp
	> Author: 
	> Mail: 
	> Created Time: 2019年03月05日 星期二 19时29分52秒
 ************************************************************************/

#include "common.h"

int main(){
    char file[20] = "./conf_log";
    char buffer[100] = {0};
    FILE *fp = fopen(file, "r");
//    fread(buffer, sizeof(buffer), 1, fp);
    FILE *fp2 = fopen("./temp", "w");
//    fwrite(buffer, sizeof(buffer), 1, fp2);
    while(fgets(buffer, sizeof(buffer), fp)){
        printf("%s", buffer);
    }   
//    write_Pi_log(file, "%s\n","hello");
    return 0;
}
