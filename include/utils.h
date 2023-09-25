#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static inline void error(const char *fmt, ...) {
	fprintf(stderr, "[%s:%s:%d] ", __FILE__, __func__, __LINE__);
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, fmt, args);
	va_end(args);
}

static inline void *safemalloc(const size_t n) {
	void *p = malloc(n);
	if (p == NULL) {
		error("Error: could not allocate %ld bytes\n", n);
		exit(-1);
	}
	return p;
}

#endif	// UTILS_H_INCLUDED