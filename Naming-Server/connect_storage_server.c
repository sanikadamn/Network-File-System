#include "includes.h"

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
File* files[10000];
Server* servers[100];

/**
 * IP:
 * NPORT:
 * CPORT:
 * NUMFILES:
 * FILENAME:
 * FILESIZE:
 **/

int filecount = 0;
int servercount = 0;

void* getFileInfo(void* arg) {
	// get the file paths from the storage server
	// assuming that the storage server sends the files as an array of file
	// paths
	Server* storage = (Server*)arg;

	// read one file packet
	buf_t file_packet;
	buf_malloc(&file_packet, sizeof(str_t), 2048);
    int numberfiles = 0;
	while (1) {
		int bytes_read = recv(storage->server_socket, file_packet.data,
		                      sizeof(buf_t), MSG_PEEK);
		if (bytes_read < 0) {
			perror("read");
			break;
		}
		if (bytes_read == 0) {
			printf("Done reading\n");
			break;
		}
		// convert packet to struct
		buf_t* ip = read_str(storage->server_socket, "IP:");
		buf_t* nport = read_str(storage->server_socket, "NPORT:");
		buf_t* cport = read_str(storage->server_socket, "CPORT:");
		i32 numfiles = read_i32(storage->server_socket, "NUMFILES:");
		buf_t* filename = read_str(storage->server_socket, "FILENAME:");
		i64 filesize = read_i64(storage->server_socket, "FILESIZE:");

		// check if the file is already in the array
		int file_exists = 0;
        pthread_mutex_lock(&file_lock);
		for (int i = 0; i < filecount; i++) {
			if (strcmp(files[i]->filename,
			           CAST(char, filename->data)) == 0) {
				for (int j = 0; j < 3; j++) {
					if (files[i]->storageserver_socket[j] == -1) 
                    {
                        strcpy(files[i]->ss_ip[j], CAST(char, ip->data));
                        strcpy(files[i]->ns_port[j], CAST(char, nport->data));
                        strcpy(files[i]->client_port[j], CAST(char, cport->data));
						files[i]->storageserver_socket[j] = storage->server_socket;
						files[i]->storageserver[j] = storage->server_addr;
						break;
					}
				}
				file_exists = 1;
				break;
			}
		}
		if (file_exists == 0) {

			// store this file in the file array
			strcpy(files[filecount]->ss_ip[0], CAST(char, ip->data));
			strcpy(files[filecount]->ns_port[0],
			       CAST(char, nport->data));
			strcpy(files[filecount]->client_port[0],
			       CAST(char, cport->data));
			files[filecount]->num_files = numfiles;
			strcpy(files[filecount]->filename,
			       CAST(char, filename->data));
			files[filecount]->filesize = filesize;
			for (int i = 0; i < 3; i++)
				files[filecount]->storageserver_socket[i] = -1;

			files[filecount]->storageserver[0] =
			    storage->server_addr;
			files[filecount]->storageserver_socket[0] =
			    storage->server_socket;
			filecount++;
		}
        // find what server the file is in in the server array
        int serverindex = -1;
        for(int i = 0; i < servercount; i++)
        {
            if(servers[i]->server_socket == storage->server_socket)
            {
                serverindex = i;
                break;
            }
        }
        // add the file to the server array
        servers[serverindex]->filesize += filesize;
		pthread_mutex_unlock(&file_lock);
        numberfiles++;
        if(numberfiles == numfiles)
            break;
	}

	return NULL;
}

void* connectStorageServer(void* arg) {
	// allocate memory for each file
	for (int i = 0; i < 10000; i++) {
		files[i] = (File*)malloc(sizeof(File));
	}
	while (1) {
		// get connections from servers and store them
		int connfd;
		struct sockaddr_in storage_server;
		socklen_t len = sizeof(storage_server);
		connfd = accept(NS_storage->server_socket,
		                (struct sockaddr*)&storage_server, &len);
		if (connfd < 0) {
			perror("accept");
			// exit(0);
		} else
			printf("Connection accepted from %s:%d.\n",
			       inet_ntoa(storage_server.sin_addr),
			       ntohs(storage_server.sin_port));

		// make a thread for the storage server to accept the initial
		// data
		Server* storage = (Server*)malloc(sizeof(Server));
        servers[servercount] = malloc(sizeof(Server));
        servers[servercount]->server_socket = connfd;
        servers[servercount]->server_addr = storage_server;
        servercount++;
		storage->server_socket = connfd;
		storage->server_addr = storage_server;
		tpool_work(thread_pool, (void (*)(void*))getFileInfo,
		           (void*)storage);
		// pthread_create(&get_file_info, NULL, getFileInfo, (void
		// *)storage);
	}
}