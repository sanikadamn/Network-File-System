#include "includes.h"

void ss_connect(buf_t *ns_res)
{
    i32 port = read_i32(ns_res->data, "PORT:");
    char *ip = read_str(ns_res->data, "IP:");

    client_ss_socket = init_connection(ip, port);
}

int ss_read_req(char *action, char *file)
{
    packet_a req;
    strcpy(req.action, action);
    strcpy(req.filename, file);

    int len = MAX_ACTION_LENGTH+MAX_FILENAME_LENGTH+20;
    char request[len];

    sprintf(request, "ACTION:%s\nFILENAME:%s", req.action, req.filename);

    // send request to ss
    if(send(client_ss_socket, request, len, 0) < 0)
    {
        perror("[-] send error");
        exit(0);
    }

    // receive feedback from ss
    char feedback[MAX_FEEDBACK_STRING_LENGTH+20];
}