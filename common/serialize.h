#ifndef SERIALISE
#define SERIALISE

#include <stddef.h>

#include "buffer.h"

/**
 * Serialises strings to remove any special characters and replaces with hex values.
 *  */


/**
 * Returns a buffer with a serialised string, where all non-printable characters are
 * replaced by hex values. Is a buffer because it will be reused for transmitting packets
 * */
str_t* serialize_string(const char* src);

char* deserialize_string(const char* buffer);

#endif // SERIALISE
