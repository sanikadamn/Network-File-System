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
int len = MAX_ACTION_LENGTH + MAX_FILENAME_LENGTH + 20;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

void log_it(char *msg)
{
	pthread_mutex_lock(&log_lock);
	FILE *log = fopen("../log.txt", "a");
	write(log, msg, strlen(msg));
	write(log, "\n", 1);
	fclose(log);
	pthread_mutex_unlock(&log_lock);
}

void* getFileInfo(void* arg) {
	// get the file paths from the storage server
	// assuming that the storage server sends the files as an array of file
	// paths
	Server* storage = (Server*)arg;

	// read one file packet
	while (1) {
		int err;
		char *ip = read_line(storage->server_socket, 50, &err);
		if (err == -1) {
			perror("ns receive");
			return NULL;
		}
		log_it(ip);
		char ip_addr[50];
		sscanf(ip, "IP:%s", ip_addr);
		free(ip);

		char *np = read_line(storage->server_socket, 10, &err);
		if (err == -1) {
			perror("ns receive");
			return NULL;
		}
		log_it(np);
		int nport;
		sscanf(np, "NPORT:%d", &nport);
		free(np);

		char *cp = read_line(storage->server_socket, 10, &err);
		if (err == -1) {
			perror("ns receive");
			return NULL;
		}
		log_it(cp);
		int cport;
		sscanf(cp, "CPORT:%d", &cport);
		free(cp);

		char *nf = read_line(storage->server_socket, 20, &err);
		if (err == -1) {
			perror("ns receive");
			return NULL;
		}
		log_it(nf);
		int numfiles;
		sscanf(nf, "NUMFILES:%d", &numfiles);
		free(nf);

		printf("Received heartbeat from %s:%d", ip_addr, nport);
		printf("\tClient port: %d\n", cport);

		storage->cport = cport;
		storage->nport = nport;

		while(numfiles --)
		{
			char *filename_header = read_line(storage->server_socket, MAX_FILENAME_LENGTH, &err);
			if (err == -1) {
				perror("ns receive");
				return NULL;
			}
			log_it(filename_header);
			char filename[MAX_FILENAME_LENGTH];
			sscanf(filename_header, "FILENAME:%s", filename);
			free(filename_header);

			printf("File received: %s\n", filename);

			char *fs = read_line(storage->server_socket, 20, &err);
			if (err == -1) {
				perror("ns receive");
				return NULL;
			}
			log_it(fs);
			int filesize;
			sscanf(fs, "FILESIZE:%d", &filesize);
			free(fs);

			// check if the file already exists
			pthread_mutex_lock(&file_lock);
			int fileexists = 0;
			for(int i = 0; i < filecount && !fileexists; i ++)
			{
				if(strcmp(files[i]->filename, filename) == 0)
				{
					fileexists = 1;
					int assigned = 0;
					for (int j = 0; j < COPY_SERVERS; j++)
					{
						if (files[i]->on_servers[j] == NULL) continue;

						// TODO: correct this info
						if(files[i]->on_servers[j]->server_socket == storage->server_socket)
						{
							assigned = 1;
							break;
						}

						if (assigned == 0 && files[i]->on_servers[j]->server_socket == -1)
						{
							files[i]->on_servers[j]->server_socket = storage->server_socket;
							files[i]->on_servers[j]->server_addr = storage->server_addr;
							break;
						}
					}
				}
			}
			if (fileexists == 0)
			{
				files[filecount]->deleted = 0;
				strcpy(files[filecount]->filename, filename);
				files[filecount]->on_servers[0] = storage;
				for (int j = 1; j < COPY_SERVERS; j ++)
				{
					files[filecount]->on_servers[j] = NULL;
				}
				filecount++;
				printf("New file created on the server!\n");
			}
			for(int i = 0; i < servercount; i++)
			{
				if(servers[i]->server_socket == storage->server_socket)
				{
					servers[i]->filesize += filesize;
					break;
				}
			}
			pthread_mutex_unlock(&file_lock);
		}

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
			printf("Connection accepted from storage server %s:%d.\n",
			       inet_ntoa(storage_server.sin_addr),
			       ntohs(storage_server.sin_port));
		log_it(strcat("Connection accepted from ", inet_ntoa(storage_server.sin_addr)));

		// make a thread for the storage server to accept the initial
		// data
		Server* storage = (Server*)malloc(sizeof(Server));
		pthread_mutex_lock(&file_lock);
        servers[servercount] = malloc(sizeof(Server));
        servers[servercount]->server_socket = connfd;
        servers[servercount]->server_addr = storage_server;
        servercount++;
		pthread_mutex_unlock(&file_lock);
		pthread_mutex_init(&storage->ss_lock, NULL);
		storage->server_socket = connfd;
		storage->server_addr = storage_server;
		tpool_work(thread_pool, (void (*)(void*))getFileInfo,
		           (void*)storage);
	}
}

// int send_create_delete_to_ss(char *action, char *filepath)
// {
// 	// select a storage server to create the file on
// 	int ss_index = rand() % servercount;
// 	Server* ss = servers[ss_index];
// 	// send the create command to the storage server
	
// 	packet_a req;
// 	strcpy(req.action, action);
// 	strcpy(req.filename, filepath);

// 	char request[len];

// 	sprintf(request, "ACTION:%s\nFILENAME:%s\n%n", req.action,
// 	        req.filename, &len);

// 	// send request to ss
// 	if (send(ss->server_socket, request, len, 0) < 0) {
// 		printf("[-] send error");
// 		return 0;
// 	}

// 	// receive feedback from ss
// 	char* feedback;
// 	packet_c fb;
// 	feedback = read_line(ss->server_socket, MAX_FEEDBACK_STRING_LENGTH + 20);
// 	sscanf(feedback, "STATUS:%d", &fb.status);
// 	// free(feedback);

// 	if (fb.status == 0) {
// 		printf("[-] %d operation unsuccessful\n", action);
// 		return 0;
// 	}

// 	// send acknowledgement to client
// 	char ack[100];
// 	sprintf(ack, "STATUS:%d\n%n", fb.status, &len);
// 	if (send(NS_client->server_socket, ack, len, 0) < 0) {
// 		printf("[-] feedback send error");
// 		return 0;
// 	}
// }
