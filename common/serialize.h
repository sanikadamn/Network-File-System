#ifndef SERIALISE
#define SERIALISE

#include <stddef.h>

/**
 * Serialises strings to remove any special characters and replaces with hex values.
 *  */

char* serialize_string(const char* src);

char* deserialize_string(const char* buffer);

#endif // SERIALISE
