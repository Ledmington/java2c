#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>

void error(const char *fmt, ...);

void *safemalloc(const size_t n);

#endif	// UTILS_H_INCLUDED