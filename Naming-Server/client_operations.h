#ifndef CONNECTCLIENTTONS_H
#define CONNECTCLIENTTONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

typedef struct req {
    int req_type;
    char path[1024];
    char location[1024];
} Request;

typedef struct res{
    int errortype;
    struct sockaddr_in server_addr;
    int server_socket;
} Response;



void *connectClientToNS(void *arg);
void *clientRequests(void *arg);
int find_file(char path[]);
int write_tofile(int fd);
int read_fromfile(int fd);
int delete_file(int fd);
int create_file(int fd);
int moreinfo_file(int fd);
int list_file(int fd);
int read_write(int fd, int read);
int copy_file(int fd);

#endif