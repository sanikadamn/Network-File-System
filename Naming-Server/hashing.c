#include "includes.h"

int hash(char path[]) {
	int hash = 5381;
	int i = 0;

	while (path[i] != '\0') {
		hash = ((hash << 5) + hash) + (char)path[i];
		i++;
	}

	return hash % MAX_FILES;
}

int find_file_hashing(File* file) {
	// hash file path
	int hash_value = hash(file->filename);

	// check if file exists
	if (files[hash_value] == NULL) {
		return -1;
	}

	// check if file is deleted
	if (files[hash_value]->deleted == 1) {
		return -1;
	}

	// check if file name matches

	if (strcmp(files[hash_value]->filename, file->filename) != 0) {
		// use linear probing to find the file
		int i = hash_value + 1;
		while (i != hash_value) {
			if (files[i] == NULL) {
				return -1;
			}
			if (files[i]->deleted == 0 &&
			    strcmp(files[i]->filename, file->filename) == 0) {
				return i;
			}
			i = (i + 1) % MAX_FILES;
		}
	}

	return hash_value;
}

int insert_file_hashing(File* file) {
	// hash file path
	int hash_value = hash(file);
	if (files[hash_value] != NULL) {
        files[hash_value] = file;
		return -1;
	}
    // find a space which is empty using linear probing
    int i = hash_value + 1;
    while (i != hash_value) {
        if (files[i] == NULL) {
            files[i] = file;
            return i;
        }
        i = (i + 1) % MAX_FILES;
    }
    return 0;
}