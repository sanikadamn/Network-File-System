#include "includes.h"
#include <sys/types.h>
#include <stdint.h>

// connect the clients to the naming server
void *connectClientToNS(void *arg)
{
    while (1)
    {
        int connfd;
        struct sockaddr_in client_addr;
        uint len = sizeof(client_addr);
        connfd = accept(NS->server_socket, (struct sockaddr*)&client_addr, &len);
        if (connfd < 0)
        {
            perror("accept");
        }
        else
            printf("Connection accepted from %s:%d.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

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
        Request *req = (Request *)malloc(sizeof(Request));
        read(client->server_socket, req, sizeof(Request));
        // switch statement to handle the request
        switch (req->req_type)
        {
            case 0:
                write_tofile(req, client);
                break;
            case 1:
                read_fromfile(req, client);
                break;
            case 2:
                delete_file(req, client);
                break;
            // case 3:
            //     create_file(req, client);
            //     break;
            // case 4:
            //     list_file(req, client);
            //     break;
            // case 5:
            //     moreinfo_file(req, client);
            // default:
                printf("Invalid request.\n");
                break;
        }

    }
}

int find_file(char path[])
{
    // return the index in the array of the file
    for (int i = 0; i < filecount; i++)
    {
        if (strcmp(files[i].path, path) == 0)
            return i;
    }
    return -1;
}

int write_tofile(Request *req, Server *client)
{
    // for writing, check if the user has permissions + if the file exists
    pthread_mutex_lock(&file_lock);
    int index = find_file(req->path);
    if (filecount == 0)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        Response *res = (Response *)malloc(sizeof(Response));
        res->errortype = ENOSERV;
        int err = send(client->server_socket, res, sizeof(Response), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if(index == -1)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        Response *res = (Response *)malloc(sizeof(Response));
        res->errortype = ENOTFOUND;
        int err = send(client->server_socket, res, sizeof(Response), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if(files[index].write_perm == 0)
    {
        pthread_mutex_unlock(&file_lock);
        Response *res = (Response *)malloc(sizeof(Response));
        res->errortype = ENOPERM;
        int err = send(client->server_socket, res, sizeof(Response), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    // if the file exists and the user has permissions, send the data to the client
    Response *res = (Response *)malloc(sizeof(Response));
    res->errortype = NO_ERROR;
    res->server_addr = files[index].storageserver;
    res->server_socket = files[index].storageserver_socket;
    int err = send(client->server_socket, res, sizeof(Response), 0);
    if (err < 0)
        perror("send");
    pthread_mutex_unlock(&file_lock);

    return 0;
}


int read_fromfile(Request *req, Server *client)
{
    // for reading, check if the user has permissions + if the file exists
    pthread_mutex_lock(&file_lock);
    int index = find_file(req->path);
    if (filecount == 0)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        Response *res = (Response *)malloc(sizeof(Response));
        res->errortype = ENOSERV;
        int err = send(client->server_socket, res, sizeof(Response), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if(index == -1)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        Response *res = (Response *)malloc(sizeof(Response));
        res->errortype = ENOTFOUND;
        int err = send(client->server_socket, res, sizeof(Response), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if(files[index].read_perm == 0)
    {
        pthread_mutex_unlock(&file_lock);
        Response *res = (Response *)malloc(sizeof(Response));
        res->errortype = ENOPERM;
        int err = send(client->server_socket, res, sizeof(Response), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    // if the file exists and the user has permissions, send the data to the client
    Response *res = (Response *)malloc(sizeof(Response));
    res->errortype = NO_ERROR;
    res->server_addr = files[index].storageserver;
    res->server_socket = files[index].storageserver_socket;
    int err = send(client->server_socket, res, sizeof(Response), 0);
    if (err < 0)
        perror("send");
    pthread_mutex_unlock(&file_lock);

    return 0;
}

int delete_file(Request *req, Server *client)
{
    // for deleting, check what servers the files are in and send delete commands to all of them
    // find all instances of the file in the file array, set the bit to 1, and send delete commands to all the ss
    // if the file is not found, send error to the client
    pthread_mutex_lock(&file_lock);
    for (int i = 0; i< filecount; i++)
    {
        Command *cmd = (Command *)malloc(sizeof(Command));
        cmd->type = DELETE;
        strcpy(cmd->path, req->path);
        if (strcmp(files[i].path, req->path) == 0)
        {
            files[i].deleted = 1;
            // send delete command to the storage server
            int err = send(files[i].storageserver_socket, cmd, sizeof(Command), 0);
            if (err < 0)
                perror("send");
        }
    }
    pthread_mutex_unlock(&file_lock);
    return 0;
}

// int create_file()
// {

// }

int list_file(Request *req, Server *client)
{
    pthread_mutex_lock(&file_lock);
    // the client sends the base path thing that they want, need to check which other strings that is a substring of 
    // and send the list of those files to the client
    
    pthread_mutex_unlock(&file_lock);
}

// int moreinfo_file()
// {

// }
