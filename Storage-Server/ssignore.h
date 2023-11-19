#ifndef SSIGNORE
#define SSIGNORE

#include "../common/buffer.h"

/**
 * Read file given by file and compile all regexes present. Stores the result in
 * a buffer that is later used to check if the files are valid or no
*/
void compile_regexs (char* file);

/**
 * Checks if file is accessible by the storage server.
 * Uses regex patterns stored in .ssignore
 */
int is_accessible(char* filename);


/**
 * Frees all regexes present
 * */
void free_regexs ();

#endif // SSIGNORE
