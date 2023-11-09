#include <unistd.h>
#include <stdio.h>

#include "constants.h"
#include "filemap.h"
#include "thread_pool.h"
#include "network.h"

int usage (int argc, char* argv[]) {
    fprintf(stderr, "invalid usage!\n");

    fprintf(stderr, "Usage: %s fspath ns_ip ns_port (got ", argv[0]);
    for (int i = 0; i < argc; i++) printf("%s ", argv[i]);
    printf(")\n");

    return 1;
}

int check_access (char* path) {
    if (access(path, F_OK | R_OK | W_OK)) {
        fprintf(stderr, "Unable to obtain read/write access on given file path\nExiting!\n");
        return 1;
    }
    return 0;
}

void init_server (char* root, char* ns_ip, char* ns_port) {
    init_filemaps(root);

    send_init(ns_ip, ns_port);

    tpool_t* thread_pool = tpool_create(NUM_THREADS);

    // Handle heartbeat
    char* ns_details[2] = {ns_ip, ns_port};
    tpool_work(thread_pool, send_heartbeat, ns_details);

    tpool_work(thread_pool, listen_connections, thread_pool);
}

int main (int argc, char* argv[]) {
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
