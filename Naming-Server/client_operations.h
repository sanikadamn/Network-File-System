#ifndef CONNECTCLIENTTONS_H
#define CONNECTCLIENTTONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

void *connectClientToNS(void *arg);
void *clientRequests(void *arg);
int find_file(char path[]);
int write_tofile(Request *req, Server *client);
int read_fromfile(Request *req, Server *client);


typedef struct req {
    int req_type;
    char path[1024];
} Request;

typedef struct res{
    int errortype;
    struct sockaddr_in server_addr;
    int server_socket;
} Response;

#endif