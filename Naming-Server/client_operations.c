#include "includes.h"
#include <sys/types.h>
#include <stdint.h>

/**
REQUEST: 
FILENAME:
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
        buf_t client_packet;
        buf_malloc(&client_packet, sizeof(str_t), 2048);
        Request *req = (Request *)malloc(sizeof(Request));

        int bytes_read = recv(client->server_socket, client_packet.data, sizeof(buf_t), MSG_PEEK);
        if (bytes_read < 0)
        {
            perror("read");
            break;
        }
        else if (bytes_read == 0)
        {
            printf("Connection closed by client\n");
            break;
        }

        // convert packet to struct
        i32 req_type = read_i32(client->server_socket, "REQUEST:");
        buf_t *path = read_str(client->server_socket, "FILENAME:");
        // store the request in the request struct
        req->req_type = req_type;
        strcpy(req->path, CAST(char, path->data));


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

int read_write(Request *req, Server *client, int read)
{
    // for writing, check if the user has permissions + if the file exists
    pthread_mutex_lock(&file_lock);
    int index = find_file(req->path);
    buf_t packet;
    struct buffer buf;
    buf_malloc(&buf, sizeof(str_t), 2048);
    buf_malloc(&packet, sizeof(str_t), 2048);
    

    if (filecount == 0)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "ENOSERV");
        add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", "");
        add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
        add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", "");
        add_str_header(&CAST(str_t, packet.data)[4], "IP:", "");
        add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", "");

        coalsce_buffers(&buf, &packet);
        int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if(index == -1)
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "ENOTFOUND");
        add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", "");
        add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
        add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", "");
        add_str_header(&CAST(str_t, packet.data)[4], "IP:", "");
        add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", "");
        coalsce_buffers(&buf, &packet);
        int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    if((read == 0 && files[index]->write_perm == 0) || (read == 1 && files[index]->read_perm == 0))
    {
        // send error to the client
        pthread_mutex_unlock(&file_lock);
        add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "ENOPERM");
        add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", "");
        add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
        add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", "");
        add_str_header(&CAST(str_t, packet.data)[4], "IP:", "");
        add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", "");
        coalsce_buffers(&buf, &packet);
        int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
        if (err < 0)
            perror("send");
        return -1;
    }
    // if the file exists and the user has permissions, send the data to the client
    add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "NO_ERROR");
    add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", "");
    add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
    add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", "");
    add_str_header(&CAST(str_t, packet.data)[4], "IP:", files[index]->ss_ip);
    add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", files[index]->client_port);
    coalsce_buffers(&buf, &packet);
    int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
    if (err < 0)
        perror("send");
    pthread_mutex_unlock(&file_lock);

    return 0;
}

int write_tofile(Request *req, Server *client)
{
    return read_write(req, client, 0);
}


int read_fromfile(Request *req, Server *client)
{
    return read_write(req, client, 1);
}

int delete_file(Request *req, Server *client)
{
    // for deleting, check what servers the files are in and send delete commands to all of them
    // find all instances of the file in the file array, set the bit to 1, and send delete commands to all the ss
    // if the file is not found, send error to the client
    pthread_mutex_lock(&file_lock);
    for (int i = 0; i< filecount; i++)
    {        
        if (strcmp(files[i]->filename, req->path) == 0)
        {
            buf_t packet;
            struct buffer buf;
            buf_malloc(&buf, sizeof(str_t), 2048); // not rew
            buf_malloc(&packet, sizeof(str_t), 2048); // sixe = 6
            add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "DELETE");
            add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", "");
            add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
            add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", req->path);
            add_str_header(&CAST(str_t, packet.data)[4], "IP:", "");
            add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", "");
            coalsce_buffers(&buf, &packet);
            files[i]->deleted = 1;
            // send delete command to the storage server
            int err = send(files[i]->storageserver_socket, &buf, sizeof(buf_t), 0); // CAST(char, buf->data), buf->len
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
    File *uniquefiles[10000];
    int unique = 0;
    for (int i = 0; i < filecount; i ++)
    {
        if(strncmp(files[i]->filename, req->path, strlen(req->path) == 0))
        {
            int isunique = 0;
            for(int j = 0;j<unique; j++)
            {
                if(strcmp(uniquefiles[j]->filename, files[i]->filename) == 0)
                {
                    isunique = 1;
                    break;
                }
            }
            if(isunique == 0)
            {
                uniquefiles[unique] = (File *)malloc(sizeof(File));
                uniquefiles[unique] = files[i];
                unique++;
                // send the file to the client
                buf_t packet;
                struct buffer buf;
                buf_malloc(&buf, sizeof(str_t), 2048);
                buf_malloc(&packet, sizeof(str_t), 2048);

                add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "");
                add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", files[i]->filesize);
                add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
                add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", files[i]->filename);
                add_str_header(&CAST(str_t, packet.data)[4], "IP:", "");
                add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", "");
                coalsce_buffers(&buf, &packet);
                int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
                if(err < 0)
                    perror("send");
            }

        }
    }
    buf_t packet;
    struct buffer buf;
    buf_malloc(&buf, sizeof(str_t), 2048);
    buf_malloc(&packet, sizeof(str_t), 2048);

    add_str_header(&CAST(str_t, packet.data)[0], "STATUS:", "DONE");
    add_str_header(&CAST(str_t, packet.data)[1], "FILESIZE:", "");
    add_str_header(&CAST(str_t, packet.data)[2], "PERMISSIONS:", "");
    add_str_header(&CAST(str_t, packet.data)[3], "FILENAME:", "");
    add_str_header(&CAST(str_t, packet.data)[4], "IP:", "");
    add_str_header(&CAST(str_t, packet.data)[5], "CPORT:", "");
    coalsce_buffers(&buf, &packet);
    int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
    if(err < 0)
        perror("send");
    pthread_mutex_unlock(&file_lock);
    return 0;
}

int moreinfo_file(Request *req, Server *client)
{
    // get the file size and permissions
    pthread_mutex_lock(&file_lock);
    int index = find_file(req->path);
    if()
    buf_t packet;
    struct buffer buf;
    buf_malloc(&buf, sizeof(str_t), 2048);
    buf_malloc(&packet, sizeof(str_t), 2048);
    add_str_header(&CAST(str_t, packet.data)[0], "REQUEST:", "");
    add_int_header(&CAST(str_t, packet.data)[1], "FILESIZE:", files[index]->filesize);
    add_str_header(&CAST(str_t, packet.data)[2], "FILENAME:", files[index]->filename);
    add_int_header(&CAST(str_t, packet.data)[3], "READ:", files[index]->read_perm);
    coalsce_buffers(&buf, &packet);
    int err = send(client->server_socket, &buf, sizeof(buf_t), 0);
    if(err < 0)
        perror("send");

    pthread_mutex_unlock(&file_lock);
    return 0;
}
