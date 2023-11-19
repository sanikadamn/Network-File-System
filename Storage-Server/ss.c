#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "filemap.h"
#include "network.h"
#include "ssignore.h"
#include "../common/thread_pool.h"

struct files* ss_files;

struct network net_details;

int usage(int argc, char* argv[]) {
	fprintf(stderr, "invalid usage!\n");

	fprintf(stderr, "Usage: %s fspath ns_ip ns_port (got ", argv[0]);
	for (int i = 0; i < argc; i++)
		printf("%s ", argv[i]);
	printf(")\n");

	return 1;
}

int check_access(char* path) {
	if (access(path, F_OK | R_OK | W_OK)) {
		fprintf(stderr, "Unable to obtain read/write access on given "
		                "file path\nExiting!\n");
		return 1;
	}
	return 0;
}

void init_server(char* root) {
	ss_files = init_ss_filemaps(root);

	// Handle heartbeat
	int ns = init_connection(net_details.ns_ip, net_details.ns_port, 0);
	if (ns == -1)
		return;

	int client_socket =
	    init_connection(net_details.ss_ip, net_details.client_port, 1);
	if (client_socket == -1)
		return;

	tpool_t* thread_pool = tpool_create(NUM_THREADS);

	tpool_work(thread_pool, send_heartbeat, (void*)ns);

	struct listen_args args_client = {thread_pool, client_socket};
	tpool_work(thread_pool, listen_connections, (void*)&args_client);

	tpool_wait(thread_pool);
	printf("No more connections left. Closing the server\n");
	tpool_destroy(thread_pool);
	free_regexs();
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		return usage(argc, argv);
	}

	char* root_path = argv[1];
	char* ns_ip = argv[2];
	char* ns_port = argv[3];

	strcpy(net_details.ss_ip, LOCALHOST);
	strcpy(net_details.ns_ip, ns_ip);
	strcpy(net_details.client_port, DEFAULT_CLIENT_PORT);
	strcpy(net_details.ns_port, ns_port);

	if (check_access(root_path)) {
		return 1;
	}

	fprintf(stderr, "Starting storage server!\n");

	init_server(root_path);
}
