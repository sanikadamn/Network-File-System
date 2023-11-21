#ifndef FILEMAP__
#define FILEMAP__


// Required to use the correct POSIX version while compiling with C99 standards
#include <pthread.h>
#if __STDC_VERSION__ >= 199901L
# define _XOPEN_SOURCE 600
#else
# define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <stdint.h>
#include <time.h>
#include <semaphore.h>

#include "../common/buffer.h"
#include "ssignore.h"

/*
** Stores file metadata.
**
** The three semaphores are to solve the reader/writer problem.
**
*/
struct file_metadata {
    str_t remote_filename;
    str_t local_filename;
    size_t file_size;
    int deleted;
    int updated;
    int perms;

    pthread_rwlock_t rwlock;
};

struct files {
    struct buffer files; // type: file_metadata
    pthread_rwlock_t rwlock;
};

extern struct files* ss_files;

/*
** The local file name is unique. The goal is to be easy to search and sort.
** Right now, however, we have a simple hex translation. Since we only deal with
** hex pathnames, we can also use unicode. This also helps to deal with uniqueness
**
** Why not store the entire tree? On a networked file system, files in the same folder
** may exist on different drives. Recreating the folders would end up becoming impossibly
** hard for deeply nested folders. Instead, the folders are recreated on NameNodes.
*/
void retrieve_local_filename  (str_t* local_filename, const str_t* remote_filename);
void retrieve_remote_filename (str_t* remote_filename, const str_t* local_filename);


/*
** Initialise file mappings. This should be run only on startup, and reads the filesystem
** to obtain all files on the system.
*/
void file_maps_alloc (struct files* file_maps);

void insert_file_map (struct files* file_maps, struct file_metadata new_file);
void delete_file_map (struct files* file_maps, str_t remote_filename);

void free_file_maps (struct files* file_maps);

/*
 * Fills file_data with all the files that are accesible by the ss_info.
 *
 */
void get_files (buf_t* file_data, char* path);

struct files* init_ss_filemaps(char* path);

struct file_metadata* search_file (char* remote_path);

void add_file (char* local_filename, char* remote_filename, int num_bytes, int perms);
#endif
