#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "types.h"

/*
Reads 1 unsigned byte from content and increments the idx.
*/
u1 read_u1(const u1 *content, const uint32_t length, uint32_t *idx);

/*
Reads 2 unsigned bytes from content in big-endian and increments the idx.
*/
u2 read_u2(const u1 *content, const uint32_t length, uint32_t *idx);

/*
Reads 4 unsigned bytes from content in big-endian and increments the idx.
*/
u4 read_u4(const u1 *content, const uint32_t length, uint32_t *idx);

/*
Reads N unsigned bytes from content in big-endian and increments the idx.
*/
void read_n(const u1 *content, const uint32_t length, uint32_t *idx, void *p,
			const u4 n);

#endif	// INPUT_H_INCLUDED