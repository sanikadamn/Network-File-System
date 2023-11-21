#include <sys/socket.h>
#include <stdlib.h>

#include "client.h"

char* readline(int fd, int max_len, int* err) {
    char* str = malloc(sizeof(char) * max_len);
    if (str == NULL) return str;
    char* head = str;

	char ch;
	while ((*err = recv(fd, &ch, sizeof(char), 0)) > 0) {
		if (ch == '\n' || (head - str) == max_len) {
            *head = '\0';
			break;
		}
        *head++ = ch;
	}

    int len = head - str;
    str = realloc(str, len);
    return str;
}

void respond (int fd) {

}
