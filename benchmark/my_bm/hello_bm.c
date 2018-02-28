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
    char *mapped_data, *mapped_data2;
    int devfd;
    int khee;
    int lock_return;

    char new_str[4096 *1];

    int i;
    for(i=0; i<(4095*1); i++){
        new_str[i] = 'o';
    }

    new_str[4095] = '\0';

    devfd = open("/dev/npheap",O_RDWR);
    if(devfd < 0) {
        fprintf(stderr, "Device open failed");
        exit(1);
    }

    size = 4096;

    mapped_data = (char *)npheap_alloc(devfd,5,size);
    lock_return = npheap_lock(devfd, 5);

    printf("\n\n Lock Return (First) : %d\n", lock_return);

    strcpy(mapped_data, &new_str);

    scanf("%d", &khee);

    mapped_data2 = (char *)npheap_alloc(devfd,7,size);
    lock_return = npheap_lock(devfd, 7);

    printf("\n\n Lock Return (Second) : %d\n", lock_return);

    //npheap_unlock(devfd, 5);

    npheap_unlock(devfd, 7);



    return 0;
}