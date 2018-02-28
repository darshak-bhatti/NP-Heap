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

#define LOCK_UNLOCK_ALLOC  2
#define LOCK_UNLOCK_DELETE  3
#define LOCK_UNLOCK_GETSIZE 4
#define LOCK_UNLOCK_DELETE_GETSIZE 5
#define NO_WRITE_LOCK_UNLOCK_DELETE  6
#define NO_WRITE_LOCK_UNLOCK_GETSIZE 7
#define NO_WRITE_LOCK_UNLOCK_DELETE_GETSIZE 8

//#define DEBUG 1

int main(int argc, char *argv[])
{
    int i=0,number_of_processes = 1, number_of_objects=1024, max_size_of_objects = 8192 ,j, feature_combination = 0; 
    int a;
    int pid = 1; // This should be initialized to make sure the for loop is executed.
    int size = 0;
    char data[8192];
    char filename[256];
    char *mapped_data = NULL;
    int devfd;
    unsigned long long msec_time;
    FILE *fp;
    struct timeval current_time;
    //if(argc < 3)
    //{
    //    fprintf(stderr, "Usage: %s number_of_objects max_size_of_objects number_of_processes\n",argv[0]);
    //    exit(1);
    //}

#ifdef DEBUG
    printf("argc: %d\n", argc);
#endif
    if(argc != 5)
    {
        fprintf(stderr, "Usage: %s number_of_objects max_size_of_objects\n",argv[0]);
        exit(1);
    }
    number_of_objects = atoi(argv[1]);
    max_size_of_objects = atoi(argv[2]);
    feature_combination = atoi(argv[3]);
    number_of_processes = atoi(argv[4]);
    devfd = open("/dev/npheap",O_RDWR);
    if(devfd < 0)
    {
        fprintf(stderr, "Device open failed");
        exit(1);
    }
    // Writing to objects
    for(i=0;i<(number_of_processes-1) && pid != 0;i++)
    {
        pid=fork();
        srand((int)time(NULL)+(int)getpid());
    }
    // printf("Pid: %d\n", (int)getpid());
    sprintf(filename,"npheap.%d.log",(int)getpid());
    fp = fopen(filename,"w");
    if (fp == NULL)
    {
        printf("Log file open failed.\n");
	exit(-1);
    }
    
    // This will test the alloc feature.
    if (feature_combination == LOCK_UNLOCK_ALLOC)
    {
   #ifdef DEBUG
        printf("DEBUG: This is the entrance of feature combination LOCK_UNLOCK_ALLOC.\n");
   #endif
        for(i = 0; i < number_of_objects; i++)
        {
            npheap_lock(devfd,i);
            while(size ==0 || size <= 10)
            {
                size = rand() % max_size_of_objects;
            }
            mapped_data = (char *)npheap_alloc(devfd,i,size);
            if(!mapped_data)
            {
                fprintf(stderr,"Failed in npheap_alloc()\n");
                exit(1);
            }
            memset(mapped_data, 0, size);
            a = rand()+1;
            gettimeofday(&current_time, NULL);
            for(j = 0; j < size-10; j=strlen(mapped_data))
            {
                sprintf(mapped_data,"%s%d",mapped_data,a);
            }
            fprintf(fp,"S\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
            npheap_unlock(devfd,i);
        }
   #ifdef DEBUG
        printf("DEBUG: This is the end of feature combination LOCK_UNLOCK_ALLOC.\n");
   #endif
    }

    // For three of the feature definition, no data will be written to the heap.
    if ((feature_combination == LOCK_UNLOCK_DELETE) || (feature_combination == LOCK_UNLOCK_GETSIZE) || (feature_combination == LOCK_UNLOCK_DELETE_GETSIZE))
    {
   #ifdef DEBUG
        printf("DEBUG: This is the alloc for feature combination 3, 4, 5.\n");
   #endif
        for(i = 0; i < number_of_objects; i++)
        {
            npheap_lock(devfd,i);
            size = npheap_getsize(devfd,i);
            while(size ==0 || size <= 10)
            {
                size = rand() % max_size_of_objects;
            }
            mapped_data = (char *)npheap_alloc(devfd,i,size);
            if(!mapped_data)
            {
                fprintf(stderr,"Failed in npheap_alloc()\n");
                exit(1);
            }
            memset(mapped_data, 0, size);
            a = rand()+1;
            gettimeofday(&current_time, NULL);
            for(j = 0; j < size-10; j=strlen(mapped_data))
            {
                sprintf(mapped_data,"%s%d",mapped_data,a);
            }
            fprintf(fp,"S\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
            npheap_unlock(devfd,i);
        }
    }
    
#ifdef DEBUG
    printf("Before the delete code segment.\n");
#endif
    // try delete something
    if ((feature_combination == NO_WRITE_LOCK_UNLOCK_DELETE) || (feature_combination == LOCK_UNLOCK_DELETE) || (feature_combination == NO_WRITE_LOCK_UNLOCK_DELETE_GETSIZE) || (feature_combination == LOCK_UNLOCK_DELETE_GETSIZE))
    {
#ifdef DEBUG
    printf("Before rand.\n");
#endif
        i = rand() % number_of_objects;
#ifdef DEBUG
    printf("After rand, before lock.\n");
#endif
        npheap_lock(devfd, i);
#ifdef DEBUG
    printf("Before delete.");
#endif
        gettimeofday(&current_time, NULL);
        npheap_delete(devfd, i);
#ifdef DEBUG
    printf("After delete.");
#endif
        fprintf(fp,"D\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
        npheap_unlock(devfd, i);
    }
    // try get size
    else if ((feature_combination == NO_WRITE_LOCK_UNLOCK_GETSIZE) || (feature_combination == LOCK_UNLOCK_GETSIZE) || (feature_combination == NO_WRITE_LOCK_UNLOCK_DELETE_GETSIZE) || (feature_combination == LOCK_UNLOCK_DELETE_GETSIZE))
    {
        i = rand() % number_of_objects;
        npheap_lock(devfd, i);
        gettimeofday(&current_time, NULL);
        size = npheap_getsize(devfd, i);
	if (size != 0)
        {
            mapped_data = (char *)npheap_alloc(devfd,i,size);
            fprintf(fp,"G\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
        }
        npheap_unlock(devfd, i);

    }
    close(devfd);
#ifdef DEBUG
    printf("The benchmark program is going to exit.");
#endif
    
    fclose(fp);

    if (pid != 0)
        for (int j = 0; j < number_of_processes - 1; j++)
            wait(NULL);

    return 0;
}

