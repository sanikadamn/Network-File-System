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

packet_d ns_expect_redirect(char *action, char *file)
{
    packet_d rd = {0};
    if(strcasecmp(action, "read") == 0 || strcasecmp(action, "write") == 0 || strcasecmp(action, "info") == 0)
    {
        packet_a req;
        strcpy(req.action, action);
        strcpy(req.filename, file);

        char request[MAX_STR_LENGTH];
        int len;

        sprintf(request, "ACTION:%s\nFILENAME:%s\n%n", req.action, req.filename, &len);

        // send request to ns
        if(send(client_ns_socket, request, len, 0) < 0)
        {
            perror("[-] send error");
            exit(0);
        }

        // expect a redirect packet (type d)
        char *redirect;
        redirect = read_line(client_ns_socket, MAX_STR_LENGTH+20);
        sscanf(redirect, "STATUS:%d", &rd.status);

        if(rd.status == ENOTFOUND)
        {
            printf("[-] File not found\n");
            return rd;
        }
        if(rd.status == ENOSERV)
        {
            printf("[-] No servers active\n");
            return rd;
        }
        if(rd.status == ENOPERM)
        {
            printf("[-] You do not have permission to access this file\n");
            return rd;
        }

        redirect = read_line(client_ns_socket, MAX_STR_LENGTH+20);
        sscanf(redirect, "IP:%s", rd.ip);

        redirect = read_line(client_ns_socket, MAX_STR_LENGTH+20);
        sscanf(redirect, "PORT:%d", &rd.port);

        // print everything
        printf("NS redirected client to SS at %s:%d\n", rd.ip, rd.port);
        free(redirect);
    }
    return rd;
}

void ns_expect_feedback(char *action, char *file1, char *file2)
{
    if(strcasecmp(action, "create") == 0 || strcasecmp(action, "delete") == 0 || strcasecmp(action, "copy") == 0)
    {
        packet_a req;
        strcpy(req.action, action);
        strcpy(req.filename, file1);

        char request[MAX_STR_LENGTH];
        int len;
        
        if(strcasecmp(action, "copy") != 0)
            sprintf(request, "ACTION:%s\nFILENAME:%s\n%n", req.action, req.filename, &len);
        else 
            sprintf(request, "ACTION:%s\nFILENAME:%s\nDESTINATION:%s\n%n", req.action, req.filename, file2, &len);

        // send request to ns
        if(send(client_ns_socket, request, len, 0) < 0)
        {
            perror("[-] send error");
            return;
        }

        // expect a feedback packet (type c)
        char *feedback;
        packet_c fb = {0};
        feedback = read_line(client_ns_socket, MAX_STR_LENGTH+20);
        sscanf(feedback, "STATUS:%d", &fb.status);

        if(fb.status == ENOTFOUND)
            printf("[-] File not found\n");
        else if(fb.status == EINVAL)
            printf("[-] This file cannot be created because it already exists\n");
        else if(fb.status == OK)
            printf("[+] 'File %s' action performed successfully\n", action);
        free(feedback);
    }
    return;
}