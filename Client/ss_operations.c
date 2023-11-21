#include "includes.h"

void ss_connect(char *ip, int port)
{
    client_ss_socket = init_connection(ip, port);
}

void ss_read_req(char *action, char *file)
{
    packet_a req;
    strcpy(req.action, action);
    strcpy(req.filename, file);

    int len = MAX_ACTION_LENGTH+MAX_FILENAME_LENGTH+20;
    char request[len];

    sprintf(request, "ACTION:%s\nFILENAME:%s\n", req.action, req.filename);
    // zero out the rest of the buffer
    for(int i = strlen(request); i < len; i++)
        request[i] = '\0';

    // send request to ss
    if(send(client_ss_socket, request, len, 0) < 0)
    {
        perror("[-] send error");
        exit(0);
    }

    // receive feedback from ss
    char feedback[MAX_FEEDBACK_STRING_LENGTH+20];
    if(recv(client_ss_socket, feedback, MAX_FEEDBACK_STRING_LENGTH+20, 0) < 0)
    {
        perror("[-] recv error");
        exit(0);
    }

    // parse feedback
    packet_c fb;
    sscanf(feedback, "STATUS:%d\nNUMBYTES:%d\n", &fb.status, &fb.numbytes);

    if(fb.status == 0)
    {
        printf("[-] File not found\n");
        return 0;
    }

    // receive data from ss and print to stdout
    char data[fb.numbytes+20];
    while(1)
    {
        int bytes = recv(client_ss_socket, data, fb.numbytes+20, 0);
        if(bytes < 0)
        {
            perror("[-] recv error");
            exit(0);
        }
        else if(bytes == 0)
            break;
        else
            printf("%s", data);
    }
}