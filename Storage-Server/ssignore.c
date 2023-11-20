#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ssignore.h"

static buf_t regexs;

void report_regex_error(int errcode, const char* restrict err_str) {
	size_t bytes = regerror(errcode, &((regex_t*)regexs.data)[regexs.len],
	                        NULL, (size_t)0);
	char* str = malloc(sizeof(char) * bytes);
	bytes =
	    regerror(errcode, &((regex_t*)regexs.data)[regexs.len], str, bytes);
	fprintf(stderr, "%s: %s\n", err_str, str);
	free(str);
}

void compile_regexs(char* filename) {
	FILE* fp = fopen(filename, "r");

	if (fp == NULL) {
		fprintf(stderr, "Warning: Unable to open %s.\n", filename);
		goto early_exit;
	}

	char* line = NULL;
	ssize_t read;
	size_t len = 0;

	buf_malloc(&regexs, sizeof(regex_t),
	           32); // small array but not large enough to blow up
	while ((read = getline(&line, &len, fp)) != -1) {
		if (regexs.len == regexs.capacity) {
			buf_resize(&regexs, 2 * regexs.capacity);
		}
		line[strcspn(line, "\n")] = 0;

		int error = regcomp(&((regex_t*)regexs.data)[regexs.len++],
		                    line, REG_EXTENDED | REG_NOSUB);
		if (error != 0) {
			report_regex_error(
			    error, "Error compiling regex, ignoring regex");
			regexs.len--;
		}
	}

	if (ferror(fp)) {
		// TODO: Replace with warning
		fprintf(stderr, "Warning: Unable to read %s.\n", filename);
		free(line);
		line = NULL;
		goto read_early_exit;
	}
	free(line);
	line = NULL;
	return;

read_early_exit:
	free(line);
	buf_free(&regexs);
early_exit:
	buf_malloc(&regexs, sizeof(regex_t), 1);
	return;
}

int is_accessible(char* filename) {
	for (regex_t* r = &CAST(regex_t, regexs.data)[0];
	     r < &CAST(regex_t, regexs.data)[regexs.len]; r++) {
		int match = regexec(r, filename, 0, NULL, 0);
		switch (match) {
		case 0:
			return 0;
			break;
		case REG_NOMATCH:
			break;
		default:
			report_regex_error(match, "Error executing regex");
		}
	}
	return 1;
};

void free_regexs() {
	for (regex_t* r = &CAST(regex_t, regexs.data)[0];
	     r < &CAST(regex_t, regexs.data)[regexs.len]; r++) {
		free(r);
	}
	buf_free(&regexs);
}
