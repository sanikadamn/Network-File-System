#include "includes.h"
#include <sys/types.h>
#include <stdint.h>

/**
REQUEST: 
FILENAME:
LOCATION:
**/

/**
STATUS:
FILESIZE:
PERMISSIONS:
FILENAME:
IP:
CPORT:
**/
// sending stuff back to client

// connect the clients to the naming server
void *connectClientToNS(void *arg)
{
    while (1)
    {
        int connfd;
        struct sockaddr_in client_addr;
        uint32_t len = sizeof(client_addr);
        connfd = accept(NS_client->server_socket, (struct sockaddr*)&client_addr, &len);
        if (connfd < 0)
        {
            perror("accept");
        }
        else
            printf("Connection accepted from client %s:%d.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // make a thread for the client to be able to send requests
        Server *client = (Server *)malloc(sizeof(Server));
        client->server_socket = connfd;
        client->server_addr = client_addr;
        tpool_work(thread_pool, (void (*)(void *))clientRequests, (void *)client);
        // pthread_create(&client_requests, NULL, clientRequests, (void *)client);
    }
}

// separate threads for each client, the client sends requests to the ns from here
void *clientRequests(void *arg)
{
    Server *client = (Server *)arg;
    while(1)
    {
        // get the request from the client
        const char action_header[] = "ACTION:";
        int err;
        char *header = read_line(client->server_socket, MAX_ACTION_LENGTH + strlen(action_header) + 1, &err);
        if(err == -1)
            perror("[-] client request");

        // convert packet to struct
        char *action = malloc(sizeof(char) * MAX_ACTION_LENGTH);
        sscanf(header, "ACTION:%s", action);
        free(header);

        printf("action received: %s\n", action);
        // switch statement to handle the request
        if(strncmp(action, "read", 4) == 0) read_fromfile(client->server_socket);
        else if(strncmp(action, "write", 5) == 0) write_tofile(client->server_socket);
        else if(strncmp(action, "info", 4) == 0) moreinfo_file(client->server_socket);
        else if(strncmp(action, "create", 6) == 0) create_file(client->server_socket);
        else if(strncmp(action, "delete", 6) == 0) delete_file(client->server_socket);
    }
    return NULL;
}

int find_file(char path[])
{
    // return the index in the array of the file
    for (int i = 0; i < filecount; i++)
    {
        if (strcmp(files[i]->filename, path) == 0)
            return i;
    }
    return -1;
}

// read = 0 for write, 1 for read, 2 for info
int read_write(int fd, int read)
{
    // for writing, check if the user has permissions + if the file exists
    const char file_name_header[] = "FILENAME:";
    int err;
    char *header = read_line(fd, MAX_FILENAME_LENGTH + strlen(file_name_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char *filename = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    sscanf(header, "FILENAME:%s", filename);
    free(header);

    pthread_mutex_lock(&file_lock);
    int index = find_file(filename);

    if (filecount == 0 || servercount == 0)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        packet_d packet;
        packet.status = ENOSERV;
        strcpy(packet.ip, "");
        packet.port = 0;
        int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
        char request[len];
        sprintf(request, "STATUS:%d\nIP:%s\nPORT:%d\n", packet.status, packet.ip, packet.port);
        printf("No servers active\n");
        int err = send(fd, request, len, 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if(index == -1)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        packet_d packet;
        packet.status = ENOTFOUND;
        strcpy(packet.ip, "");
        packet.port = 0;
        int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
        char request[len];
        sprintf(request, "STATUS:%d\nIP:%s\nPORT:%d\n", packet.status, packet.ip, packet.port);
        printf("File not found\n");
        int err = send(fd, request, len, 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if (read == 0)
    {
        // check whether any of the servers is down
        int serverdown = 0;
        for (int i = 0; i < COPY_SERVERS; i++)
        {
            if (files[index]->on_servers[i]->server_socket == -1)
            {
                serverdown = 1;
                break;
            }
        }
        if (serverdown)
        {
            // send error to the client
            pthread_mutex_unlock(&file_lock);
            packet_d packet;
            packet.status = ENOPERM;
            strcpy(packet.ip, "");
            packet.port = 0;
            int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
            char request[len];
            sprintf(request, "STATUS:%d\nIP:%s\nPORT:%d\n", packet.status, packet.ip, packet.port);
            printf("Client does not have permission for this file\n");
            int err = send(fd, request, len, 0);
            if (err < 0)
                perror("send");
        }
    }
    // send ss data to client
    packet_d packet;
    packet.status = OK;
    char ip[INET_ADDRSTRLEN + 1];
    inet_ntop(AF_INET, &(files[index]->on_servers[0]->server_addr.sin_addr.s_addr), ip, INET_ADDRSTRLEN);
    strcpy(packet.ip, ip);
    packet.port = files[index]->on_servers[0]->server_addr.sin_port;
    int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
    char res[len];
    sprintf(res, "STATUS:%d\nIP:%s\nPORT:%d\n", packet.status, packet.ip, packet.port);
    pthread_mutex_unlock(&file_lock);

    int send_ret = send(fd, res, len, 0);
    if (send_ret < 0)
        perror("send");

    return 0;
}

int write_tofile(int fd)
{
    return read_write(fd, 0);
}


int read_fromfile(int fd)
{
    return read_write(fd, 1);
}

int delete_file(int fd)
{
    // for deleting, check what servers the files are in and send delete commands to all of them
    // find all instances of the file in the file array, set the bit to 1, and send delete commands to all the ss
    // if the file is not found, send error to the client
    const char file_name_header[] = "FILENAME:";
    int err;
    char *header = read_line(fd, MAX_FILENAME_LENGTH + strlen(file_name_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char filename[MAX_FILENAME_LENGTH];
    sscanf(header, "FILENAME:%s", filename);
    free(header);
    packet_c fb = {0};

    pthread_mutex_lock(&file_lock);
    int serverdown = 0;
    int countfiles = 0;
    for (int i = 0; i < filecount; i++)
    {        
        if (strncmp(files[i]->filename, filename, strlen(filename)) == 0)
        {
            // check if all the servers are not -1
            // TODO 
            // if any server is down, send error
            countfiles++;
            for (int j = 0; j < COPY_SERVERS; j++)
            {
                if (files[i]->on_servers[j]->server_socket == -1)
                {
                    goto send_status;
                }
            }
        }
    }
    if (countfiles == 0)
    {
        fb.status = ENOTFOUND;
    }
    else if (serverdown == 0)
    {
        // send delete command to all the servers
        for (int i = 0; i < filecount; i++)
        {
            if (strncmp(files[i]->filename, filename, strlen(filename)) == 0)
            {
                for (int j = 0; j < COPY_SERVERS; j++)
                {
                    packet_a packet;
                    strcpy(packet.action, "delete");
                    strcpy(packet.filename, filename);
                    packet.numbytes = 0;

                    int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
                    char res[len];
                    sprintf(res, "ACTION:%s\nFILENAME:%s", packet.action, packet.filename);

                    int err = send(files[i]->on_servers[j]->server_socket, res, len, 0);
                    if (err < 0)
                        perror("send");
                }
            }
        }
        fb.status = OK;
    }
    else
    {
        send_status:
            fb.status = ENOSERV;
    }
    pthread_mutex_unlock(&file_lock);
    int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
    char request[len];
    sprintf(request, "STATUS:%d\n", fb.status);
    int send_ret = send(fd, request, len, 0);
    if (send_ret < 0)
        perror("send");
    return 0;
}

int create_file(int fd)
{
    const char file_name_header[] = "FILENAME:";
    int err;
    char *header = read_line(fd, MAX_FILENAME_LENGTH + strlen(file_name_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char *filename = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    sscanf(header, "FILENAME:%s", filename);
    free(header);

    // for creating, check if the file already exists, if it does, send error to the client
    pthread_mutex_lock(&file_lock);
    int index = find_file(filename);
    if (index != -1)
    {
        packet_d packet;
        packet.status = EINVAL;
        strcpy(packet.ip, "");
        packet.port = 0;

        int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
        char res[len];
        sprintf(res, "STATUS:%d\n", packet.status);
        if (send(fd, res, len, 0) < 0)
            perror("send");
    }
    // if the file does not exist, send the create command to three storage servers 
    // sort the storage servers based on the filesize
    for (int i = 0; i < servercount; i++)
    {
        for (int j = 0; j < servercount - i - 1; j++)
        {
            if (servers[j]->filesize > servers[j + 1]->filesize)
            {
                Server *temp = servers[j];
                servers[j] = servers[j + 1];
                servers[j + 1] = temp;
            }
        }
    }
    // send the create command to the three storage servers
    for (int i = 0; i < COPY_SERVERS; i++)
    {
        packet_a packet;
        strcpy(packet.action, "create");
        strcpy(packet.filename, filename);
        packet.numbytes = 0;

        int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
        char request[len];
        sprintf(request, "ACTION:%s\nFILENAME:%s", packet.action, packet.filename);

        for (int j = 0; j < COPY_SERVERS; j++)
        {
            int err = send(servers[i]->server_socket, request, len, 0);
            if (err < 0)
                perror("send");
        }
    }

    pthread_mutex_unlock(&file_lock);
    return 0;
}

int moreinfo_file(int fd)
{
    return read_write(fd, 2);
}