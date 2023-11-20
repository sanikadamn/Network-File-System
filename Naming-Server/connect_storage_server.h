#ifndef CONNECTSERVER_H
#define CONNECTSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../common/thread_pool.h"

void *connectStorageServer(void *arg);
void *getFileInfo(void *arg);

typedef struct file_info{
    char ss_ip[3][50];
    char ns_port[3][5];
    char client_port[3][5];
    int num_files;
    char filename[1024];
    uint64_t filesize;
    struct sockaddr_in storageserver[3];
    int storageserver_socket[3];




    int write_perm; // 0 for no, 1 for yes
    int read_perm;  // 0 for no, 1 for yes
    int deleted;
} File;

extern pthread_mutex_t file_lock;
extern File *files[10000];

extern int filecount;
extern int servercount;

typedef struct serv{
    int server_socket;
    struct sockaddr_in server_addr;
    uint64_t filesize;
} Server;

// make the threadpool thing 
extern tpool_t* thread_pool;

extern Server *NS_storage;
extern Server *NS_client;
extern Server *servers[100];



#endif