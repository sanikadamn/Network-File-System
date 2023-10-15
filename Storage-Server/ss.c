#include <unistd.h>
#include <stdio.h>

int usage (int argc, char* argv[]) {
    fprintf(stderr, "invalid usage!\n");
    fprintf(stderr, "Usage: %s fspath\n", argv[0]);
    return 1;
}

int main (int argc, char* argv[]) {
    if (argc != 2) {
        return usage(argc, argv);
    }

    char* root_path = argv[1];
    if (access(root_path, F_OK | R_OK | W_OK)) {
        fprintf(stderr, "Unable to obtain read/write access on given file path\nExiting!\n");
        return 1;
    }
    fprintf(stderr, "Starting storage server!\n");
}
