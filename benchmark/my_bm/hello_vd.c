#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <npheap.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int a;
    int pid;
    int size;
    char data[8192];
    char filename[256];
    char *mapped_data;
    int devfd;

    char new_str[4096 *1];

    int i;
    for(i=0; i<(4095*1); i++){
        new_str[i] = 'G';
    }

    new_str[4095] = '\0';

    devfd = open("/dev/npheap",O_RDWR);
    if(devfd < 0) {
        fprintf(stderr, "Device open failed");
        exit(1);
    }

    size = 4096;

    mapped_data = (char *)npheap_alloc(devfd,5,size);
    printf("\n Hello World : %s \n", mapped_data);
    printf("\n Size is : %d \n", strlen(mapped_data));



    npheap_delete(devfd, 5);
    //npheap_delete(devfd, 5);
    //mapped_data = (char *)npheap_alloc(devfd,5,size);





    return 0;
}