#include "includes.h"

void ss_connect(buf_t *ns_res)
{
    i32 port = read_i32(ns_res->data, "PORT:");
    char *ip = read_str(ns_res->data, "IP:");

    client_ss_socket = init_connection(ip, port);
}

void ss_read_req(char *filepath)
{
    buf_t *request;
    buf_malloc(request, sizeof(char), 2);
    (*request).len = 2;

    // add headers
    add_str_header(&request[0], "REQUEST:", "READ");
    add_str_header(&request[1], "FILENAME:", filepath);

    // coalesce the buffers
    buf_t* packet;
    coalsce_buffers(packet, request);

    // send to storage server
    int send_ret = send(client_ss_socket, CAST(char, packet->data), packet->len, 0);
    if(send_ret < 0)
    {
        perror("[-] error sending request to storage server");
        exit(0);
    }

    buf_t response;
    buf_malloc(&response, sizeof(char), 1024);
    int status = read_i32(client_ss_socket, "STATUS:");
    if (status == SS_FILE_FOUND) {
        while(1) {
            int recv_ret = recv(client_ss_socket, CAST(char, response.data), 1024, MSG_PEEK);
            if(recv_ret < 0)
            {
                perror("[-] error receiving response from storage server");
                exit(0);
            }
            if (response.len == 0) {
                break;
            }
            printf("%s", CAST(char, response.data));
        }
    }
}