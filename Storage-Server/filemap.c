#include <semaphore.h>
#include <stdlib.h>

#include "filemap.h"

struct files* init_ss_filemaps(char* path) {
    struct files* f = malloc(sizeof(struct files));
    // TODO: Fix initial values of semaphore initialisation
    sem_init(&f->data_queue, 0, 1);
    sem_init(&f->data_read_lock, 0, 1);
    sem_init(&f->data_write_lock, 0, 1);

    buf_malloc(&f->files, sizeof(struct file_metadata), 1);

    return f;
}

buf_t* prepare_filemap_packet (const struct files file_maps) {
    buf_t* buffer = malloc(sizeof(buf_t));
    buf_malloc(buffer, 1, 0);
    return buffer;
}
