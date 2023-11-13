#ifndef FILEMAP__
#define FILEMAP__


// Required to use the correct POSIX version while compiling with C99 standards
#if __STDC_VERSION__ >= 199901L
# define _XOPEN_SOURCE 600
#else
# define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */


#include <time.h>
#include <semaphore.h>

#include "../common/buffer.h"

/*
** Stores file metadata.
**
** The three semaphores are to solve the reader/writer problem.
**
*/
struct file_metadata {
    str_t remote_filename;
    int file_size;
    struct timespec last_modified_time;
    // TODO: find a way to store permissions

    sem_t file_write_lock;
    sem_t file_read_lock;
    sem_t file_queue;
};

struct files {
    struct buffer files; // type: file_metadata

    sem_t data_write_lock;
    sem_t data_read_lock;
    sem_t data_queue;
};


/*
** The local file name is unique. The goal is to be easy to search and sort.
** Right now, however, we have a simple base64 translation. Since we only deal with
** base64 pathnames, we can also use unicode.
**
** Why use base64? It is easy to reverse to obtain the remote filename from the
** local representation of a file.
**
** Why not store the entire tree? On a networked file system, files in the same folder
** may exist on different drives. Recreating the folders would end up becoming impossibly
** hard for deeply nested folders. Instead, the folders are recreated on NameNodes.
*/
str_t* retrieve_local_filename (str_t remote_filename);
str_t* retrieve_remote_filename (str_t local_filename);


/*
** Initialise file mappings. This should be run only on startup, and reads the filesystem
** to obtain all files on the system.
*/
void file_maps_alloc (struct files* file_maps);

void insert_file_map (struct files* file_maps, struct file_metadata new_file);
void delete_file_map (struct files* file_maps, str_t remote_filename);

void free_file_maps (struct files* file_maps);

/*
** Prepare file maps into a packet that can be sent over to the naming server
*/

buf_t* prepare_filemap_packet (const struct files file_maps);

struct files* init_ss_filemaps();
#endif
