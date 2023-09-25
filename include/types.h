#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <stdint.h>

// Useful typedefs to be closer to the official specification
// https://docs.oracle.com/javase/specs/jvms/se21/html/jvms-4.html
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef int8_t i1;
typedef int16_t i2;
typedef int32_t i4;
typedef int64_t i8;

// Utility union to easily read 32 bits as float
typedef union {
	i4 i;
	float f;
} int2float;

// Utility union to easily read 64 bits as double
typedef union {
	i8 i;
	double f;
} long2double;

#endif	// TYPES_H_INCLUDED