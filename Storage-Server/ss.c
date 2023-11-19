#include <stdio.h>
#include <unistd.h>

#include "constants.h"
#include "filemap.h"
#include "network.h"
#include "../common/thread_pool.h"

struct files* ss_files;

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

void init_server(char* root, char* ns_ip, char* ns_port) {
	ss_files = init_ss_filemaps(root);

	// Handle heartbeat
	int ns = init_connection(LOCALHOST, DEFAULT_NS_PORT, 0);
	if (ns == -1)
		return;

	int client_socket = init_connection(LOCALHOST, DEFAULT_CLIENT_PORT, 1);
	if (client_socket == -1)
		return;

	tpool_t* thread_pool = tpool_create(NUM_THREADS);

	tpool_work(thread_pool, send_heartbeat, (void*)ns);

	struct listen_args args_client = {thread_pool, client_socket};
	tpool_work(thread_pool, listen_connections, (void*)&args_client);

	tpool_wait(thread_pool);
	printf("No more connections left. Closing the server\n");
	tpool_destroy(thread_pool);
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		return usage(argc, argv);
	}

	char* root_path = argv[1];
	char* ns_ip = argv[2];
	char* ns_port = argv[3];

	if (check_access(root_path)) {
		return 1;
	}

	fprintf(stderr, "Starting storage server!\n");

	init_server(root_path, ns_ip, ns_port);
}
