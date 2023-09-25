#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(const char *fmt, ...) {
	fprintf(stderr, "[%s:%s:%d] ", __FILE__, __func__, __LINE__);
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, fmt, args);
	va_end(args);
}

void *safemalloc(const size_t n) {
	void *p = malloc(n);
	if (p == NULL) {
		error("Error: could not allocate %ld bytes\n", n);
		exit(-1);
	}
	return p;
}