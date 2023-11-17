#include <dirent.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "constants.h"
#include "filemap.h"

// NOLINTBEGIN(concurrency-*)
void get_files(buf_t* f, char* path) {
	DIR* dir = opendir(path);
	if (dir == NULL) {
		perror(path);
		return;
	}

	buf_malloc(f, sizeof(struct file_metadata),
	           32); // Allocates with 32 elements to prevent repeated
	                // allocations for small sizes

	struct stat st;
	struct dirent* ent;
	while ((ent = readdir(dir))) {
		// check if number has reached capacity, and if not, continue
		// filling. else resize
		if (f->capacity == f->len) {
			buf_resize(f, 2 * f->capacity);
		}

		struct file_metadata* file =
		    &((struct file_metadata*)f->data)[f->len++];

		size_t filename_len = strlen(ent->d_name);

		printf("file found: %s\n", ent->d_name);
		// Look up file properties
		int err = fstatat(dirfd(dir), ent->d_name, &st, 0);
		if (err == -1) {
			// TODO: Custom error mechanism??
			perror(ent->d_name);
			continue;
		}

		// Ignore if not a file
		if ((st.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "Ignoring as not a file\n");
			f->len--;
			continue;
		}

		// Check if it is supposed to be accessible by the storage
		// server
		if (is_accessible(ent->d_name)) {
			// Is accessible. Continue
			buf_malloc(&file->local_filename, sizeof(char),
			           filename_len + 1);
			strncpy((char*)(file->local_filename.data), ent->d_name,
			        filename_len);
			file->local_filename.len = filename_len;
			file->file_size = st.st_size;
		} else {
			fprintf(stderr, "Ignoring as not accessible\n");
			f->len--;
		}
	}
}
// NOLINTEND(concurrency-*)

struct files* init_ss_filemaps(char* path) {
	char* ssignore_path =
	    malloc(strlen(path) + strlen(DEFAULT_SSIGNORE_PATH) + 1);
	strcpy(ssignore_path, path);
	strcat(ssignore_path, DEFAULT_SSIGNORE_PATH);
	compile_regexs(ssignore_path);
	free(ssignore_path);

	struct files* f = malloc(sizeof(struct files));
	// TODO: Fix initial values of semaphore initialisation
	sem_init(&f->data_queue, 0, 1);
	sem_init(&f->data_read_lock, 0, 1);
	sem_init(&f->data_write_lock, 0, 1);

	get_files(&f->files, path);

	return f;
}

buf_t* prepare_filemap_packet(const struct files file_map) {
	buf_t* buffer = malloc(sizeof(buf_t));
	buf_malloc(buffer, 1, 0);
	return buffer;
}
