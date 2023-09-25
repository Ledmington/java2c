#include "input.h"

#include <string.h>

#include "utils.h"

u1 read_u1(const u1 *content, const uint32_t length, uint32_t *idx) {
	if (*idx >= length) {
		error("\nError: called read_u1 with overflow\n");
		exit(-1);
	}
	const u1 x = content[*idx];
	(*idx)++;
	return x;
}

u2 read_u2(const u1 *content, const uint32_t length, uint32_t *idx) {
	if (*idx + 1 >= length) {
		error("\nError: called read_u2 with overflow\n");
		exit(-1);
	}
	const u2 x = ((u2)(content[*idx]) << 8) | (u2)(content[*idx + 1]);
	*idx += 2;
	return x;
}

u4 read_u4(const u1 *content, const uint32_t length, uint32_t *idx) {
	if (*idx + 3 >= length) {
		error("\nError: called read_u4 with overflow\n");
		exit(-1);
	}
	const u4 x = ((u4)(content[*idx]) << 24) | ((u4)(content[*idx + 1]) << 16) |
				 ((u4)(content[*idx + 2]) << 8) | (u4)(content[*idx + 3]);
	*idx += 4;
	return x;
}

void read_n(const u1 *content, const uint32_t length, uint32_t *idx, void *p,
			const u4 n) {
	if (n <= 0) {
		// nothing to read
		return;
	}
	if (*idx + n - 1 >= length) {
		error("\nError: called read_n with overflow\n");
		exit(-1);
	}
	memcpy(p, &content[*idx], n);
	*idx += n;
}