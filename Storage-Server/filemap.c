#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "constants.h"
#include "filemap.h"
#include "network.h"
#include "../common/concurrency.h"

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
			f->len--;
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
			char* filepath = realpath(ent->d_name, NULL);
			filename_len = strlen(filepath);
			buf_malloc(&file->local_filename, sizeof(char),
			           filename_len + 1);
			strncpy((char*)(file->local_filename.data), ent->d_name,
			        filename_len + 1);
			file->local_filename.len = filename_len;
			file->file_size = st.st_size;

			retrieve_remote_filename(&file->remote_filename,
			                         &file->local_filename);
			file->updated = 0;
		} else {
			fprintf(stderr, "Ignoring as not accessible\n");
			f->len--;
		}
	}

	closedir(dir);
}
// NOLINTEND(concurrency-*)

void retrieve_remote_filename(str_t* remote_filename,
                              const str_t* local_filename) {
	buf_malloc(remote_filename, sizeof(char), local_filename->len / 2 + 2);

	char* dst = CAST(char, remote_filename->data);
	char* src = CAST(char, local_filename->data);

	static const char LOOKUP[128] = {
	    ['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3, ['4'] = 0x4,
	    ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7, ['8'] = 0x8, ['9'] = 0x9,
	    ['A'] = 0xA, ['B'] = 0xB, ['C'] = 0xC, ['D'] = 0xD, ['E'] = 0xE,
	    ['F'] = 0xF, ['a'] = 0xa, ['b'] = 0xb, ['c'] = 0xc, ['d'] = 0xc,
	    ['e'] = 0xe, ['f'] = 0xf};

	for (size_t i = 0; i < local_filename->len; i += 2) {
		*dst = (char)((LOOKUP[src[i]] & 0xF) << 4);
		if ((i + 1) < local_filename->len)
			*dst = (char)(*dst | LOOKUP[src[i + 1]]);
		dst++;
		remote_filename->len++;
	}

	remote_filename->len = strlen(CAST(char, remote_filename->data));
	*dst = '\0';
}

void retrieve_local_filename(str_t* local_filename,
                             const str_t* remote_filename) {
	buf_malloc(local_filename, sizeof(char), remote_filename->len * 2 + 1);

	char* src = CAST(char, remote_filename->data);
	char* dst = CAST(char, local_filename->data);

	static const char HEX_LOOKUP[] = "0123456789ABCDEF";

	for (size_t i = 0; i < remote_filename->len; ++i) {
		*dst++ = HEX_LOOKUP[(src[i] >> 4) & 0xF];
		*dst++ = HEX_LOOKUP[src[i] & 0xF];
		local_filename->len += 2;
	}
	*dst = '\0';
}

struct files* init_ss_filemaps(char* path) {
	char* ssignore_path =
	    malloc(strlen(path) + strlen(DEFAULT_SSIGNORE_PATH) + 1);
	strcpy(ssignore_path, path);
	strcat(ssignore_path, DEFAULT_SSIGNORE_PATH);
	compile_regexs(ssignore_path);
	free(ssignore_path);

	struct files* f = malloc(sizeof(struct files));

	pthread_rwlock_init(&(f->rwlock), 0);

	get_files(&f->files, path);

	return f;
}

void add_file (char* local_filename, char* remote_filename, int num_bytes, int perms) {
	WRITER_ENTER(ss_files);
	if (ss_files->files.len >= ss_files->files.capacity) {
		buf_resize(&(ss_files->files), 2 * ss_files->files.capacity);
	}

	struct file_metadata* files = CAST(struct file_metadata, ss_files->files.data);
	int filepos = ss_files->files.len;

	files[filepos].deleted = 0;
	files[filepos].file_size = 0;

	int lf_len = strlen(local_filename);
	buf_malloc(&files[filepos].local_filename, sizeof(char), lf_len + 1);
	memcpy(CAST(char, files[filepos].local_filename.data), local_filename, lf_len);
	files[filepos].local_filename.len = files[filepos].local_filename.capacity;

	int rem_len = strlen(remote_filename);
	buf_malloc(&files[filepos].remote_filename, sizeof(char), rem_len + 1);
	memcpy(CAST(char, files[filepos].remote_filename.data), remote_filename, rem_len);
	files[filepos].remote_filename.len = files[filepos].remote_filename.capacity;

	files[filepos].updated = 1;
	files[filepos].perms = perms;
	pthread_rwlock_init(&files[filepos].rwlock, 0);

	ss_files->files.len++;

	WRITER_EXIT(ss_files);
}

struct file_metadata* search_file (char* remote_path) {
	struct file_metadata* f = CAST(struct file_metadata, ss_files->files.data);
	for (int i = 0; i < ss_files->files.len; i++) {
		printf("filename: %s\n", CAST(char, f->remote_filename.data));
		if (!strcmp(remote_path, CAST(char, f->remote_filename.data)))
			return f;
		f++;
	}
	return NULL;
}
