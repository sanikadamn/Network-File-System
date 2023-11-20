#ifndef __CLIENT_NAME_SERVER_OPERATIONS_H
#define __CLIENT_NAME_SERVER_OPERATIONS_H

#include "includes.h"

typedef struct req {
    int req_type;
    char path[1024];
} Request;

typedef struct res{
    int errortype;
    struct sockaddr_in server_addr;
    int server_socket;
} Response;

buf_t *creq_to_ns(char *req, char *file);
int validate_ns_response(buf_t *response);

#endif