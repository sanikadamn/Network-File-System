#include "includes.h"

int len = MAX_ACTION_LENGTH+MAX_FILENAME_LENGTH+20;

void ss_connect(char *ip, int port)
{
    client_ss_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_ss_socket < 0)
    {
        perror("[-] socket error");
        exit(0);
    }
    else
        printf("[+] client socket created.\n");
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr.s_addr = inet_addr(ip);

    int cnnct_ret = connect(client_ss_socket, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if(cnnct_ret < 0)
    {
        perror("[-] connect error");
        exit(0);
    }
    else
        printf("[+] connected to the storage server at port %d.\n", port);
}

void ss_read_req(char *action, char *file)
{
    packet_a req;
    strcpy(req.action, action);
    strcpy(req.filename, file);

    char request[len];

    sprintf(request, "ACTION:%s\nFILENAME:%s\n%n", req.action, req.filename, &len);

    // send request to ss
    if(send(client_ss_socket, request, len, 0) < 0)
    {
        perror("[-] send error");
        exit(0);
    }

    // receive feedback from ss
    char *feedback;
    packet_c fb;
    feedback = read_line(client_ss_socket, MAX_FEEDBACK_STRING_LENGTH+20);
    sscanf(feedback, "STATUS:%d", &fb.status);
    // free(feedback);

    if (fb.status == ENOTFOUND || fb.status == EFSERROR)
    {
        printf("[-] File not found\n");
        return;
    }

    // char *feedback;
    feedback = read_line(client_ss_socket, MAX_FEEDBACK_STRING_LENGTH+20);
    sscanf(feedback, "NUMBYTES:%d", &fb.numbytes);
    // free(feedback);

    // print everything
    printf("request that was sent to ss: %s\n", request);
    printf("feedback that was received from ss: %s\n", feedback);
    printf("status: %d\n", fb.status);
    printf("numbytes: %d\n", fb.numbytes);

    // receive data from ss and print to stdout
    char data[MAX_STR_LENGTH];
    int num_bytes = fb.numbytes;
    while(num_bytes > 0)
    {
        int bytes = recv(client_ss_socket, data, MAX_STR_LENGTH, 0);
        if(bytes < 0)
        {
            perror("[-] recv error");
            exit(0);
        }
        else if(bytes == 0)
            break;
        else {
            num_bytes -= bytes;
            printf("received data of size %d, remaining: %d\n", bytes, num_bytes);
        }
    }
}

void ss_info_req(char *action, char *file)
{
    packet_a req;
    strcpy(req.action, action);
    strcpy(req.filename, file);

    char request[len];

    sprintf(request, "ACTION:%s\nFILENAME:%s\n%n", req.action, req.filename, &len);

    // send request to ss
    if(send(client_ss_socket, request, len, 0) < 0)
    {
        perror("[-] send error");
        exit(0);
    }

    // receive feedback from ss
    char *feedback;
    packet_c fb;
    feedback = read_line(client_ss_socket, MAX_FEEDBACK_STRING_LENGTH+20);
    sscanf(feedback, "STATUS:%d", &fb.status);
    // free(feedback);

    if (fb.status == ENOTFOUND)
    {
        printf("[-] File not found\n");
        return;
    }

    // char *feedback;
    feedback = read_line(client_ss_socket, MAX_FEEDBACK_STRING_LENGTH+20);
    sscanf(feedback, "SIZE:%d", &fb.numbytes);

    feedback = read_line(client_ss_socket, MAX_FEEDBACK_STRING_LENGTH+20);
    sscanf(feedback, "PERM:%d", &fb.permissions);

    // print everything
    printf("request that was sent to ss: %s\n", request);
    printf("feedback that was received from ss: %s\n", feedback);
    printf("status: %d\n", fb.status);
    printf("numbytes: %d\n", fb.numbytes);
}

void ss_write_req(char *action, char *file)
{
    packet_a req;
    strcpy(req.action, action);
    strcpy(req.filename, file);

    char request[len];

    req.numbytes = 1000;
    sprintf(request, "ACTION:%s\nFILENAME:%s\nNUMBYTES:%d\n%n", req.action, req.filename, req.numbytes, &len);


    // send request to ss
    if(send(client_ss_socket, request, len, 0) < 0)
    {
        perror("[-] send error in request");
        exit(0);
    }

    // send data to ss 
    char data[MAX_STR_LENGTH];
    for(int i=0; i<req.numbytes; i++)
        data[i] = 'a';
    data[req.numbytes] = '\0';
    if(send(client_ss_socket, data, req.numbytes, 0) < 0)
    {
        perror("[-] send error in data");
        exit(0);
    }
}
