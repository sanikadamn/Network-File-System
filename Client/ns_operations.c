#include "ns_operations.h"

/**
** the packet that the client sends to the NS
** REQUEST: 
** FILENAME:
**/

/**
** the packet that the NS sends back to the client
** STATUS:
** FILESIZE:
** PERMISSIONS:
** FILENAME:
** IP:
** CPORT:
**/


buf_t *creq_to_ns(char *req, char *file)
{
    // send a client request to the name server
    buf_t *request;
    buf_malloc(request, sizeof(char), 2);
    (*request).len = 2;
    
    // add headers
    add_str_header(&request[0], "REQUEST:", req);
    add_str_header(&request[1], "FILENAME:", file);
    

    // coalesce the buffers
    buf_t *packet;
    coalsce_buffers(packet, request);

    // send to name server
    int send_ret = send(client_ns_socket, CAST(char, request->data), request->len, 0);
    if(send_ret < 0)
    {
        perror("[-] error sending request to name server");
        exit(0);
    }

    // receive response from name server
    buf_t *response;
    buf_malloc(response, sizeof(char), 1024);
    int recv_ret = recv(client_ns_socket, CAST(char, response), 1024, MSG_PEEK);
    if(recv_ret < 0)
    {
        perror("[-] error receiving response from name server");
        exit(0);
    }

    // return the response
    return response;
}

// int validate_ns_response(buf_t *response)
// {
//     int status = read_i32(client_ns_socket, "STATUS:");
//     if(status == ENOSERV)
//     {
//         printf("[-] no servers exist.\n");
//     }
//     else if(status == ENOTFOUND)
//     {
//         printf("[-] file not found.\n");
//     }
//     else if(status == ENOPERM)
//     {
//         printf("[-] you do not have permission to access this file.\n");
//     }
//     else if(status == NO_ERROR)
//     {
//         printf("[+] file found.\n");
//         return 0;
//     }
//     else
//     {
//         printf("[-] unknown error.\n");
//     }

//     return -1;
// }