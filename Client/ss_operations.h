#ifndef __SS_OPERATIONS_H
#define __SS_OPERATIONS_H

#include "includes.h"

typedef struct res{
    int errortype;
    struct sockaddr_in server_addr;
    int server_socket;
} Response;

void ss_connect(buf_t *ns_res);
void ss_read_req(char *filepath);
void ss_write_req(char *filepath);
void ss_info_req(char *filepath);

#endif