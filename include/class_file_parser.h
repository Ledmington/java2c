#ifndef CLASS_FILE_PARSER_H_INCLUDED
#define CLASS_FILE_PARSER_H_INCLUDED

#include <stdarg.h>
#include <stdint.h>
#include <string.h>	 // memcpy

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

#ifndef J2C_PACKED
#define J2C_PACKED __attribute((packed))
#endif

#ifndef J2C_RESTRICT
#define J2C_RESTRICT __restrict__
#endif

typedef enum J2C_PACKED {
	CONSTANT_Class = 7,
	CONSTANT_Fieldref = 9,
	CONSTANT_Methodref = 10,
	CONSTANT_InterfaceMethodref = 11,
	CONSTANT_String = 8,
	CONSTANT_Integer = 3,
	CONSTANT_Float = 4,
	CONSTANT_Long = 5,
	CONSTANT_Double = 6,
	CONSTANT_NameAndType = 12,
	CONSTANT_Utf8 = 1,
	CONSTANT_MethodHandle = 15,
	CONSTANT_MethodType = 16,
	CONSTANT_Dynamic = 17,
	CONSTANT_InvokeDynamic = 18,
	CONSTANT_Module = 19,
	CONSTANT_Package = 20
} cp_info_tag;

typedef struct J2C_PACKED {
	cp_info_tag tag;
	u1 *info;
} cp_info;

typedef enum J2C_PACKED {
	ACC_PUBLIC = 0x0001,
	ACC_PRIVATE = 0x0002,
	ACC_PROTECTED = 0x0004,
	ACC_STATIC = 0x0008,
	ACC_FINAL = 0x0010,
	ACC_SUPER = 0x0020,
	ACC_SYNCHRONIZED = 0x0020,
	ACC_VOLATILE = 0x0040,
	ACC_BRIDGE = 0x0040,
	ACC_TRANSIENT = 0x0080,
	ACC_VARARGS = 0x0080,
	ACC_NATIVE = 0x0100,
	ACC_INTERFACE = 0x0200,
	ACC_ABSTRACT = 0x0400,
	ACC_STRICT = 0x0800,
	ACC_SYNTHETIC = 0x1000,
	ACC_ANNOTATION = 0x2000,
	ACC_ENUM = 0x4000,
	ACC_MODULE = 0x8000
} access_flag;

typedef struct J2C_PACKED {
	u2 attribute_name_index;
	u4 attribute_length;
	u1 *info;
} attribute_info;

typedef struct J2C_PACKED {
	access_flag access_flags;
	u2 name_index;
	u2 descriptor_index;
	u2 attributes_count;
	attribute_info *attributes;
} field_info;

typedef struct J2C_PACKED {
	access_flag access_flags;
	u2 name_index;
	u2 descriptor_index;
	u2 attributes_count;
	attribute_info *attributes;
} method_info;

typedef struct J2C_PACKED {
	u4 magic_number;
	u2 minor_version;
	u2 major_version;
	u2 constant_pool_count;
	cp_info *constant_pool;
	access_flag access_flags;
	u2 this_class;
	u2 super_class;
	u2 interfaces_count;
	u2 *interfaces;
	u2 fields_count;
	field_info *fields;
	u2 methods_count;
	method_info *methods;
	u2 attributes_count;
	attribute_info *attributes;
} class_file;

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

/*
Reads 1 unsigned byte from content and increments the idx.
*/
static inline u1 read_u1(const u1 *J2C_RESTRICT content, const uint32_t length,
						 uint32_t *J2C_RESTRICT idx) {
	if (*idx >= length) {
		error("\nError: called read_u1 with overflow\n");
		exit(-1);
	}
	const u1 x = content[*idx];
	(*idx)++;
	return x;
}

/*
Reads 2 unsigned bytes from content in big-endian and increments the idx.
*/
static inline u2 read_u2(const u1 *J2C_RESTRICT content, const uint32_t length,
						 uint32_t *J2C_RESTRICT idx) {
	if (*idx + 1 >= length) {
		error("\nError: called read_u2 with overflow\n");
		exit(-1);
	}
	const u2 x = ((u2)(content[*idx]) << 8) | (u2)(content[*idx + 1]);
	*idx += 2;
	return x;
}

/*
Reads 4 unsigned bytes from content in big-endian and increments the idx.
*/
static inline u4 read_u4(const u1 *J2C_RESTRICT content, const uint32_t length,
						 uint32_t *J2C_RESTRICT idx) {
	if (*idx + 3 >= length) {
		fprintf(stderr, "\nError: called read_u4 with overflow\n");
		exit(-1);
	}
	const u4 x = ((u4)(content[*idx]) << 24) | ((u4)(content[*idx + 1]) << 16) |
				 ((u4)(content[*idx + 2]) << 8) | (u4)(content[*idx + 3]);
	*idx += 4;
	return x;
}

/*
Reads N unsigned bytes from content in big-endian and increments the idx.
*/
static inline void read_n(const u1 *J2C_RESTRICT content, const uint32_t length,
						  uint32_t *J2C_RESTRICT idx, void *J2C_RESTRICT p,
						  const u4 n) {
	if (n <= 0) {
		// nothing to read
		return;
	}
	if (*idx + n - 1 >= length) {
		fprintf(stderr, "\nError: called read_n with overflow\n");
		exit(-1);
	}
	memcpy(p, &content[*idx], n);
	*idx += n;
}

// QUESTION: should this be returning void?
static void read_cp_info(const u1 *J2C_RESTRICT content, const uint32_t length,
						 uint32_t *J2C_RESTRICT idx,
						 cp_info *J2C_RESTRICT cp_entry) {
	const cp_info_tag tag = read_u1(content, length, idx);
	u1 *info = NULL;

	// here no interpretation/check of the bit patterns is performed
	switch (tag) {
		case CONSTANT_Class:
			info = safemalloc(sizeof(u1) * 2);
			read_n(content, length, idx, info, 2);
			printf(" - CONSTANT_Class #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			break;
		case CONSTANT_String:
			info = safemalloc(sizeof(u1) * 2);
			read_n(content, length, idx, info, 2);
			printf(" - CONSTANT_String #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			break;
		case CONSTANT_MethodType:
			info = safemalloc(sizeof(u1) * 2);
			read_n(content, length, idx, info, 2);
			printf(" - CONSTANT_MethodType #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			break;
		case CONSTANT_Fieldref:
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			printf(" - CONSTANT_Fieldref\n");
			printf(" - class         #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			printf(" - name_and_type #%d\n",
				   ((((u2)info[2]) << 8) | (u2)info[3]));
			break;
		case CONSTANT_Methodref:
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			printf(" - CONSTANT_Methodref\n");
			printf(" - class         #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			printf(" - name_and_type #%d\n",
				   ((((u2)info[2]) << 8) | (u2)info[3]));
			break;
		case CONSTANT_InterfaceMethodref:
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			printf(" - CONSTANT_InterfaceMethodref\n");
			printf(" - class         #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			printf(" - name_and_type #%d\n",
				   ((((u2)info[2]) << 8) | (u2)info[3]));
			break;
		case CONSTANT_Integer:
			printf(" - CONSTANT_Integer\n");
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			{
				const u4 x = (((u4)info[0]) << 24) | (((u4)info[1]) << 16) |
							 (((u4)info[2]) << 8) | ((u4)info[3]);
				printf(" - value: %d\n", *(i4 *)&x);
			}
			break;
		case CONSTANT_Float:
			printf(" - CONSTANT_Float\n");
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			{
				const u4 x = (((u4)info[0]) << 24) | (((u4)info[1]) << 16) |
							 (((u4)info[2]) << 8) | ((u4)info[3]);
				printf(" - value: %f\n", *(float *)&x);
			}
			break;
		case CONSTANT_NameAndType:
			printf(" - CONSTANT_NameAndType\n");
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			printf(" - name       #%d\n", ((((u2)info[0]) << 8) | (u2)info[1]));
			printf(" - descriptor #%d\n", ((((u2)info[2]) << 8) | (u2)info[3]));
			break;
		case CONSTANT_InvokeDynamic:
			printf(" - CONSTANT_InvokeDynamic\n");
			info = safemalloc(sizeof(u1) * 4);
			read_n(content, length, idx, info, 4);
			printf(" - bootstrap_method_attr_index #%d\n",
				   ((((u2)info[0]) << 8) | (u2)info[1]));
			printf(" - name_and_type_index         #%d\n",
				   ((((u2)info[1]) << 8) | (u2)info[3]));
			break;
		case CONSTANT_Long:
			printf(" - CONSTANT_Long\n");
			info = safemalloc(sizeof(u1) * 8);
			read_n(content, length, idx, info, 8);
			{
				const u8 x = (((u8)info[0]) << 56) | (((u8)info[1]) << 48) |
							 (((u8)info[2]) << 40) | (((u8)info[3]) << 32) |
							 (((u8)info[4]) << 24) | (((u8)info[5]) << 16) |
							 (((u8)info[6]) << 8) | ((u8)info[7]);
				printf(" - value: %ld\n", *(i8 *)&x);
			}
			break;
		case CONSTANT_Double:
			printf(" - CONSTANT_Double\n");
			info = safemalloc(sizeof(u1) * 8);
			read_n(content, length, idx, info, 8);
			{
				const u8 x = (((u8)info[0]) << 56) | (((u8)info[1]) << 48) |
							 (((u8)info[2]) << 40) | (((u8)info[3]) << 32) |
							 (((u8)info[4]) << 24) | (((u8)info[5]) << 16) |
							 (((u8)info[6]) << 8) | ((u8)info[7]);
				printf(" - value: %f\n", *(double *)&x);
			}
			break;
		case CONSTANT_Utf8:
			const u2 utf8_nbytes = read_u2(content, length, idx);
			printf(" - CONSTANT_Utf8\n");
			info = safemalloc(sizeof(u2) + sizeof(u1) * utf8_nbytes);
			read_n(content, length, idx, info,
				   sizeof(u2) + sizeof(u1) * utf8_nbytes);
			printf(" - \"");
			for (u2 i = 0; i < utf8_nbytes; i++) {
				if (info[i] >= 0x01 && info[i] <= 0x7f) {
					printf("%c", info[i]);
				} else {
					const u2 x = (((u2)info[i]) << 8) | ((u2)info[i + 1]);
					if (x == 0x0000 || (x >= 0x0080 && x <= 0x07ff)) {
						printf("%c", ((((u2)info[i]) & 0x1f) << 6) +
										 (((u2)info[i + 1]) & 0x3f));
					} else {
						const u4 y = ((((u4)info[i])) << 16) |
									 (((u4)info[i + 1]) << 8) |
									 ((u4)info[i + 2]);
						if (y >= 0x0800 && y <= 0xffff) {
							printf("%c", ((((u4)info[i]) & 0xf) << 12) +
											 ((((u4)info[i + 1]) & 0x3f) << 6) +
											 (((u4)info[i + 2]) & 0x3f));
						}
					}
				}
			}
			printf("\"\n");
			break;
		case CONSTANT_MethodHandle:
			printf(" - CONSTANT_MethodHandle\n");
			info = safemalloc(sizeof(u1) * 3);
			read_n(content, length, idx, info, 3);
			break;
		default:
			fprintf(stderr, "\nError: Unknown cp_info tag %d (0x%02X)\n", tag,
					tag);
			// *idx += 2;
			exit(-1);
			return;
	}

	cp_entry->tag = tag;
	cp_entry->info = info;
}

// QUESTION: should this be returning void?
static void read_attribute_info(const u1 *J2C_RESTRICT content,
								const uint32_t length,
								uint32_t *J2C_RESTRICT idx,
								attribute_info *J2C_RESTRICT attribute) {
	attribute->attribute_name_index = read_u2(content, length, idx);
	attribute->attribute_length = read_u4(content, length, idx);
	attribute->info = safemalloc(attribute->attribute_length * sizeof(u1));
	read_n(content, length, idx, attribute->info, attribute->attribute_length);
}

// QUESTION: should this be returning void?
static void read_field_info(const u1 *J2C_RESTRICT content,
							const uint32_t length, uint32_t *J2C_RESTRICT idx,
							field_info *J2C_RESTRICT field) {
	field->access_flags = read_u2(content, length, idx);
	field->name_index = read_u2(content, length, idx);
	field->descriptor_index = read_u2(content, length, idx);
	field->attributes_count = read_u2(content, length, idx);
	field->attributes =
		safemalloc(field->attributes_count * sizeof(attribute_info));

	for (u2 i = 0; i < field->attributes_count; i++) {
		read_attribute_info(content, length, idx, &field->attributes[i]);
	}
}

// QUESTION: should this be returning void?
static void read_method_info(const u1 *J2C_RESTRICT content,
							 const uint32_t length, uint32_t *J2C_RESTRICT idx,
							 method_info *J2C_RESTRICT method) {
	method->access_flags = read_u2(content, length, idx);
	method->name_index = read_u2(content, length, idx);
	method->descriptor_index = read_u2(content, length, idx);
	method->attributes_count = read_u2(content, length, idx);
	method->attributes =
		safemalloc(method->attributes_count * sizeof(attribute_info));

	for (u2 i = 0; i < method->attributes_count; i++) {
		read_attribute_info(content, length, idx, &method->attributes[i]);
	}
}

/*
Parses the 'content' array (containing the contents of a class file)
and returns an allocated and initialized class_file struct.
*/
static class_file *parse_class_file(const u1 *J2C_RESTRICT content,
									const uint32_t length) {
	if (content == NULL || length <= 0) {
		return NULL;
	}

	uint32_t idx = 0;

	const u4 magic_number = read_u4(content, length, &idx);
	const u2 minor_version = read_u2(content, length, &idx);
	const u2 major_version = read_u2(content, length, &idx);
	const u2 constant_pool_count = read_u2(content, length, &idx);

	cp_info *constant_pool =
		(cp_info *)safemalloc((constant_pool_count - 1) * sizeof(cp_info));

	for (u2 i = 0; i < constant_pool_count - 1; i++) {
		printf("Constant pool entry #%d\n", i + 1);
		read_cp_info(content, length, &idx, &constant_pool[i]);
	}

	const u2 access_flags = read_u2(content, length, &idx);
	const u2 this_class = read_u2(content, length, &idx);
	const u2 super_class = read_u2(content, length, &idx);
	const u2 interfaces_count = read_u2(content, length, &idx);
	u2 *interfaces = safemalloc(interfaces_count * sizeof(u2));

	for (u2 i = 0; i < interfaces_count; i++) {
		interfaces[i] = read_u2(content, length, &idx);
	}

	const u2 fields_count = read_u2(content, length, &idx);
	field_info *fields = safemalloc(fields_count * sizeof(field_info));

	for (u2 i = 0; i < fields_count; i++) {
		read_field_info(content, length, &idx, &fields[i]);
	}

	const u2 methods_count = read_u2(content, length, &idx);
	method_info *methods = safemalloc(methods_count * sizeof(method_info));

	for (u2 i = 0; i < methods_count; i++) {
		read_method_info(content, length, &idx, &methods[i]);
	}

	const u2 attributes_count = read_u2(content, length, &idx);
	attribute_info *attributes =
		safemalloc(attributes_count * sizeof(attribute_info));

	for (u2 i = 0; i < attributes_count; i++) {
		read_attribute_info(content, length, &idx, &attributes[i]);
	}

	printf("Magic : %d (0x%08X)\n", magic_number, magic_number);
	printf("Minor : %d (0x%04X)\n", minor_version, minor_version);
	printf("Major : %d (0x%04X)\n", major_version, major_version);
	printf("%d constant pool entries\n", constant_pool_count);
	printf("Access flags : 0x%04X\n", access_flags);
	printf("%d interfaces\n", interfaces_count);
	printf("%d fields\n", fields_count);
	printf("%d methods\n", methods_count);
	printf("%d attributes\n", attributes_count);

	class_file *cf = safemalloc(sizeof(class_file));

	cf->magic_number = magic_number;
	cf->minor_version;
	cf->major_version = major_version;
	cf->constant_pool_count = constant_pool_count;
	cf->constant_pool = constant_pool;
	cf->access_flags = access_flags;
	cf->this_class = this_class;
	cf->super_class = super_class;
	cf->interfaces_count = interfaces_count;
	cf->interfaces = interfaces;
	cf->fields_count = fields_count;
	cf->fields = fields;
	cf->methods_count = methods_count;
	cf->methods = methods;
	cf->attributes_count = attributes_count;
	cf->attributes = attributes;

	return cf;
}

/*
Takes as input the name of the .class file to read.
Reads the .class file fully, if successfull, and returns a
pointer to a class_file structure allocated dinamically.
This class_file needs to be deallocated with class_file_destroy().
*/
class_file *read_class_file(const char *J2C_RESTRICT filename) {
	if (filename == NULL) {
		fprintf(stderr, "\nError: Could not open NULL filename\n");
		return NULL;
	}

	// open file
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "\nError: Could not open class file \"%s\"\n",
				filename);
		return NULL;
	}

	// get file length
	fseek(fp, 0L, SEEK_END);
	const uint32_t file_length = ftell(fp);

	printf("Reading file \"%s\" (%d bytes)\n", filename, file_length);

	rewind(fp);

	// allocating buffer
	u1 *file_content = (u1 *)malloc(file_length * sizeof(u1));
	if (file_content == NULL) {
		fprintf(
			stderr,
			"\nError: Could not allocate %ld bytes to store content of file "
			"\"%s\"\n",
			file_length * sizeof(u1), filename);
		fclose(fp);
		return NULL;
	}

	const size_t bytes_read = fread(file_content, sizeof(u1), file_length, fp);
	if (bytes_read != file_length) {
		fprintf(stderr, "\nError: Could not read whole class file \"%s\"\n",
				filename);
		free(file_content);
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	class_file *cf = parse_class_file(file_content, file_length);

	free(file_content);

	return cf;
}

/*
Deallocates the memory used by the given class_file structure.
*/
void class_file_destroy(class_file *J2C_RESTRICT cf) {
	for (u2 i = 0; i < cf->constant_pool_count - 1; i++) {
		free(cf->constant_pool[i].info);
	}
	free(cf->constant_pool);
	for (u2 i = 0; i < cf->fields_count; i++) {
		for (u2 j = 0; j < cf->fields[i].attributes_count; j++) {
			free(cf->fields[i].attributes[j].info);
		}
		free(cf->fields[i].attributes);
	}
	free(cf->fields);
	for (u2 i = 0; i < cf->methods_count; i++) {
		for (u2 j = 0; j < cf->methods[i].attributes_count; j++) {
			free(cf->methods[i].attributes[j].info);
		}
		free(cf->methods[i].attributes);
	}
	free(cf->methods);
	for (u2 i = 0; i < cf->attributes_count; i++) {
		free(cf->attributes[i].info);
	}
	free(cf->attributes);
	free(cf);
}

#endif	// CLASS_FILE_PARSER_H_INCLUDED