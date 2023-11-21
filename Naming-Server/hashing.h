#ifndef HASHING_H
#define HASHING_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connect_storage_server.h"

int hash(char path[]);
int find_file_hashing(File *file);
int insert_file_hashing(File *file);


#endif