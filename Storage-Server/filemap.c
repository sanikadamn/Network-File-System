#include <stdlib.h>

#include "filemap.h"

struct files* init_ss_filemaps() {
    return NULL;
}

buf_t* prepare_filemap_packet (const struct files file_maps) {
    buf_t* buffer = malloc(sizeof(buf_t));
    buf_malloc(buffer, 1, 0);
    return buffer;
}
