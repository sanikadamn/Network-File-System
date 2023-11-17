/** Link to conversation: https://chat.openai.com/share/78af6e66-c17f-4591-8a61-66a98ef4e088.
 * Code below has been lightly edited.
 *
 *  AI-generated code for thread-safe, buffer-overflow-safe serialization, and deserialization of strings in C.
 *
 *  Prompts:
 *  1. Write code for thread-safe buffer-overflow-safe serialization and deserialization of strings in C.
 *  2. Make sure that it handles pathological cases.
 *  3. Make sure non-printable characters are appropriately escaped (and unescaped) using hex representations.
 *  4. Do not write the main function.
 *  5. Allocate a new string inside the function instead of taking it as a parameter.
 *  6. Escape backslashes correctly in the serialization and deserialization procedures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "serialise.h"

// Define a mutex for thread safety
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Notice: This code was AI-generated for the given prompts.

// Serialize a string into a new dynamically allocated buffer
char* serialize_string(const char* src) {
    // Acquire the mutex lock for thread safety
    pthread_mutex_lock(&mutex);

    // Calculate the required buffer size for escaping non-printable characters and backslashes
    size_t input_length = strlen(src);
    size_t escaped_length = 0;


    for (int i = 0; i < input_length; i++) {
        int escape_char = (src[i] < 32 || src[i] > 126 || src[i] == '\\');
        int escaped_length = 4 * escape_char + 1 * !escape_char;
    }

    // Allocate a new buffer for the escaped string
    char* escaped_buffer = (char*)malloc(escaped_length + 1); // +1 for null-termination

    if (escaped_buffer == NULL) {
        // TODO: Handle memory allocation failure
        fprintf(stderr, "Error: Memory allocation failed during serialization\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    // Perform the actual escaping
    size_t j = 0;
    for (size_t i = 0; i < input_length; ++i) {
        int escape_char = (src[i] < 32 || src[i] > 126 || src[i] == '\\');
        if (src[i] < 32 || src[i] > 126 || src[i] == '\\') {
            // Non-printable character or backslash, escape it
            snprintf(&escaped_buffer[j], 5, "\\x%02X", src[i]);
        } else {
            // Copy printable character as is
            escaped_buffer[j] = src[i];
        }
        j += 4 * escape_char + 1 * !escape_char;
    }

    escaped_buffer[j] = '\0'; // Ensure null-termination

    // Release the mutex lock
    pthread_mutex_unlock(&mutex);

    return escaped_buffer;
}

// Unescape a string from a buffer with hex representations into a new dynamically allocated buffer
char* deserialize_string(const char* buffer) {
    // Acquire the mutex lock for thread safety
    pthread_mutex_lock(&mutex);

    // Calculate the required buffer size for unescaping hex representations and backslashes
    size_t buffer_length = strlen(buffer);
    size_t unescaped_length = 0;

    for (size_t i = 0; i < buffer_length; ++i) {
        if (buffer[i] == '\\' && i + 3 < buffer_length && buffer[i + 1] == 'x') {
            // Found hex representation, needs unescaping
            unescaped_length += 1;
            i += 3; // Skip the next three characters representing the hex value
        } else if (buffer[i] == '\\' && i + 1 < buffer_length) {
            // Found backslash, needs unescaping
            unescaped_length += 1;
            i += 1; // Skip the next character
        } else {
            // Copy character as is
            unescaped_length += 1;
        }
    }

    // Allocate a new buffer for the unescaped string
    char* unescaped_buffer = (char*)malloc(unescaped_length + 1); // +1 for null-termination

    if (unescaped_buffer == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Error: Memory allocation failed during deserialization\n");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    // Perform the actual unescaping
    size_t j = 0;
    for (size_t i = 0; i < buffer_length && j < unescaped_length; ++i) {
        if (buffer[i] == '\\' && i + 3 < buffer_length && buffer[i + 1] == 'x') {
            // Found hex representation, unescape it
            char hex[3];
            hex[0] = buffer[i + 2];
            hex[1] = buffer[i + 3];
            hex[2] = '\0';
            unescaped_buffer[j] = (char)strtol(hex, NULL, 16);
            i += 3;
        } else if (buffer[i] == '\\' && i + 1 < buffer_length) {
            // Found backslash, unescape it
            unescaped_buffer[j] = buffer[i + 1];
            i += 1;
        } else {
            // Copy character as is
            unescaped_buffer[j] = buffer[i];
        }
        ++j;
    }

    unescaped_buffer[j] = '\0'; // Ensure null-termination

    // Release the mutex lock
    pthread_mutex_unlock(&mutex);

    return unescaped_buffer;
}
