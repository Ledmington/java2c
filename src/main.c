#include <stdio.h>
#include <stdlib.h>

#include "class_file_parser.h"

int main(int argc, const char** argv) {
	if (argc < 1) {
		fprintf(stderr, "Error: I need at least the name of one .class file\n");
		return EXIT_FAILURE;
	}

	const char* filename = argv[1];

	class_file* cf = read_class_file(filename);
	class_file_destroy(cf);

	return EXIT_SUCCESS;
}