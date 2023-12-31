#ifndef CLASS_FILE_PARSER_H_INCLUDED
#define CLASS_FILE_PARSER_H_INCLUDED

#include <stdio.h>

#include "input.h"
#include "types.h"
#include "utils.h"

#ifndef J2C_PACKED
#define J2C_PACKED __attribute((packed))
#endif

typedef enum J2C_PACKED {
	CONSTANT_Utf8				= 1,
	CONSTANT_Integer			= 3,
	CONSTANT_Float				= 4,
	CONSTANT_Long				= 5,
	CONSTANT_Double				= 6,
	CONSTANT_Class				= 7,
	CONSTANT_String				= 8,
	CONSTANT_Fieldref			= 9,
	CONSTANT_Methodref			= 10,
	CONSTANT_InterfaceMethodref = 11,
	CONSTANT_NameAndType		= 12,
	CONSTANT_MethodHandle		= 15,
	CONSTANT_MethodType			= 16,
	CONSTANT_Dynamic			= 17,
	CONSTANT_InvokeDynamic		= 18,
	CONSTANT_Module				= 19,
	CONSTANT_Package			= 20
} cp_info_tag;

const char *cp_info_tag_names[21] = {NULL,
									 "Utf8",
									 NULL,
									 "Integer",
									 "Float",
									 "Long",
									 "Double",
									 "Class",
									 "String",
									 "Fieldref",
									 "Methodref",
									 "InterfaceMethodref",
									 "NameAndType",
									 NULL,
									 NULL,
									 "MethodHandle",
									 "MethodType",
									 "Dynamic",
									 "InvokeDynamic",
									 "Module",
									 "Package"};

typedef struct J2C_PACKED {
	cp_info_tag tag;
	u1 *info;
} cp_info;

typedef enum J2C_PACKED {
	ACC_PUBLIC		 = 0x0001,
	ACC_PRIVATE		 = 0x0002,
	ACC_PROTECTED	 = 0x0004,
	ACC_STATIC		 = 0x0008,
	ACC_FINAL		 = 0x0010,
	ACC_SUPER		 = 0x0020,
	ACC_SYNCHRONIZED = 0x0020,
	ACC_VOLATILE	 = 0x0040,
	ACC_BRIDGE		 = 0x0040,
	ACC_TRANSIENT	 = 0x0080,
	ACC_VARARGS		 = 0x0080,
	ACC_NATIVE		 = 0x0100,
	ACC_INTERFACE	 = 0x0200,
	ACC_ABSTRACT	 = 0x0400,
	ACC_STRICT		 = 0x0800,
	ACC_SYNTHETIC	 = 0x1000,
	ACC_ANNOTATION	 = 0x2000,
	ACC_ENUM		 = 0x4000,
	ACC_MODULE		 = 0x8000
} access_flag;

static const char *get_class_access_flag_name(const u2 af) {
	if (af & ACC_PUBLIC) {
		return "public";
	}
	if (af & ACC_FINAL) {
		return "final";
	}
	if (af & ACC_SUPER) {
		return "super";
	}
	if (af & ACC_INTERFACE) {
		return "interface";
	}
	if (af & ACC_ABSTRACT) {
		return "abstract";
	}
	if (af & ACC_SYNTHETIC) {
		return "synthetic";
	}
	if (af & ACC_ANNOTATION) {
		return "annotation";
	}
	if (af & ACC_ENUM) {
		return "enum";
	}
	if (af & ACC_MODULE) {
		return "module";
	}
	return "unknown";
}

static const char *get_field_access_flag_name(const u2 af) {
	if (af & ACC_PUBLIC) {
		return "public";
	}
	if (af & ACC_PRIVATE) {
		return "private";
	}
	if (af & ACC_PROTECTED) {
		return "protected";
	}
	if (af & ACC_STATIC) {
		return "static";
	}
	if (af & ACC_FINAL) {
		return "final";
	}
	if (af & ACC_VOLATILE) {
		return "volatile";
	}
	if (af & ACC_TRANSIENT) {
		return "transient";
	}
	if (af & ACC_SYNTHETIC) {
		return "synthetic";
	}
	if (af & ACC_ENUM) {
		return "enum";
	}
	return "unknown";
}

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

// QUESTION: should this be returning void?
static void read_constant_pool(const u1 *content, const uint32_t length,
							   uint32_t *idx, cp_info *cp, u2 cp_count) {
	for (u2 i = 0; i < cp_count - 1; i++) {
		printf("#%5d: ", i + 1);

		const cp_info_tag tag = (cp_info_tag)read_u1(content, length, idx);
		u1 *info			  = NULL;

		// here no interpretation/check of the bit patterns is performed
		switch (tag) {
			case CONSTANT_Utf8: {
				const u2 utf8_nbytes = read_u2(content, length, idx);
				printf("%-20s \"", cp_info_tag_names[tag]);
				info = (u1 *)safemalloc(sizeof(u2) + sizeof(u1) * utf8_nbytes);
				read_n(content, length, idx, info, sizeof(u1) * utf8_nbytes);
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
								printf("%c",
									   ((((u4)info[i]) & 0xf) << 12) +
										   ((((u4)info[i + 1]) & 0x3f) << 6) +
										   (((u4)info[i + 2]) & 0x3f));
							}
						}
					}
				}
				printf("\"\n");
			} break;
			case CONSTANT_Integer:
				info = (u1 *)safemalloc(sizeof(u1) * 4);
				read_n(content, length, idx, info, 4);
				printf("%-20s %d\n", cp_info_tag_names[tag],
					   (((u4)info[0]) << 24) | (((u4)info[1]) << 16) |
						   (((u4)info[2]) << 8) | ((u4)info[3]));
				break;
			case CONSTANT_Float:
				info = (u1 *)safemalloc(sizeof(u1) * 4);
				read_n(content, length, idx, info, 4);
				{
					int2float tmp;
					tmp.i = (((u4)info[0]) << 24) | (((u4)info[1]) << 16) |
							(((u4)info[2]) << 8) | ((u4)info[3]);
					printf("%-20s %f\n", cp_info_tag_names[tag], tmp.f);
				}
				break;
			case CONSTANT_Long:
				info = (u1 *)safemalloc(sizeof(u1) * 8);
				read_n(content, length, idx, info, 8);
				printf("%-20s %ld\n", cp_info_tag_names[tag],
					   (((u8)info[0]) << 56) | (((u8)info[1]) << 48) |
						   (((u8)info[2]) << 40) | (((u8)info[3]) << 32) |
						   (((u8)info[4]) << 24) | (((u8)info[5]) << 16) |
						   (((u8)info[6]) << 8) | ((u8)info[7]));
				i++;
				break;
			case CONSTANT_Double:
				info = (u1 *)safemalloc(sizeof(u1) * 8);
				read_n(content, length, idx, info, 8);
				{
					long2double tmp;
					tmp.i = (((u8)info[0]) << 56) | (((u8)info[1]) << 48) |
							(((u8)info[2]) << 40) | (((u8)info[3]) << 32) |
							(((u8)info[4]) << 24) | (((u8)info[5]) << 16) |
							(((u8)info[6]) << 8) | ((u8)info[7]);
					printf("%-20s %f\n", cp_info_tag_names[tag], tmp.f);
				}
				i++;
				break;
			case CONSTANT_Class:
				info = (u1 *)safemalloc(sizeof(u1) * 2);
				read_n(content, length, idx, info, 2);
				printf("%-20s #%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]));
				break;
			case CONSTANT_String:
				info = (u1 *)safemalloc(sizeof(u1) * 2);
				read_n(content, length, idx, info, 2);
				printf("%-20s #%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]));
				break;
			case CONSTANT_Fieldref:
			case CONSTANT_Methodref:
			case CONSTANT_InterfaceMethodref:
				info = (u1 *)safemalloc(sizeof(u1) * 4);
				read_n(content, length, idx, info, 4);
				printf("%-20s #%d.#%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]),
					   ((((u2)info[2]) << 8) | (u2)info[3]));
				break;
			case CONSTANT_NameAndType:
				info = (u1 *)safemalloc(sizeof(u1) * 4);
				read_n(content, length, idx, info, 4);
				printf("%-20s #%d:#%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]),
					   ((((u2)info[2]) << 8) | (u2)info[3]));
				break;
			case CONSTANT_MethodHandle:
				info = (u1 *)safemalloc(sizeof(u1) * 3);
				read_n(content, length, idx, info, 3);
				printf("%-20s %d #%d\n", cp_info_tag_names[tag], info[0],
					   ((((u2)info[1]) << 8) | (u2)info[2]));
				break;
			case CONSTANT_MethodType:
				info = (u1 *)safemalloc(sizeof(u1) * 2);
				read_n(content, length, idx, info, 2);
				printf("%-20s #%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]));
				break;
			case CONSTANT_Dynamic:
			case CONSTANT_InvokeDynamic:
				info = (u1 *)safemalloc(sizeof(u1) * 4);
				read_n(content, length, idx, info, 4);
				printf("%-20s #%d.#%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]),
					   ((((u2)info[1]) << 8) | (u2)info[3]));
				break;
			case CONSTANT_Module:
			case CONSTANT_Package:
				info = (u1 *)safemalloc(sizeof(u1) * 2);
				read_n(content, length, idx, info, 2);
				printf("%-20s #%d\n", cp_info_tag_names[tag],
					   ((((u2)info[0]) << 8) | (u2)info[1]));
				break;
			default:
				fprintf(stderr, "\nError: Unknown cp_info tag %d (0x%02X)\n",
						tag, tag);
				exit(-1);
				return;
		}

		cp[i].tag  = tag;
		cp[i].info = info;
	}
}

// QUESTION: should this be returning void?
static void read_attribute_info(const u1 *content, const uint32_t length,
								uint32_t *idx, attribute_info *attribute) {
	attribute->attribute_name_index = read_u2(content, length, idx);
	printf(" - Name #%d\n", attribute->attribute_name_index);

	attribute->attribute_length = read_u4(content, length, idx);
	attribute->info =
		(u1 *)safemalloc(attribute->attribute_length * sizeof(u1));
	read_n(content, length, idx, attribute->info, attribute->attribute_length);
}

// QUESTION: should this be returning void?
static void read_field_info(const u1 *content, const uint32_t length,
							uint32_t *idx, field_info *field) {
	field->access_flags = (access_flag)read_u2(content, length, idx);

	for (u2 i = 0; i < 8 * sizeof(u2); i++) {
		if (field->access_flags & (1 << i)) {
			printf(" %s",
				   get_field_access_flag_name(field->access_flags & (1 << i)));
		}
	}

	field->name_index = read_u2(content, length, idx);
	printf(" name:#%d", field->name_index);

	field->descriptor_index = read_u2(content, length, idx);
	printf(" descriptor:#%d\n", field->descriptor_index);

	field->attributes_count = read_u2(content, length, idx);
	field->attributes = (attribute_info *)safemalloc(field->attributes_count *
													 sizeof(attribute_info));
	printf("\n%d attributes\n", field->attributes_count);
	printf("Attributes\n");
	for (u2 i = 0; i < field->attributes_count; i++) {
		printf(" - Attr. %d\n", i);
		read_attribute_info(content, length, idx, &field->attributes[i]);
	}
	printf("\n");
}

// QUESTION: should this be returning void?
static void read_method_info(const u1 *content, const uint32_t length,
							 uint32_t *idx, method_info *method) {
	method->access_flags	 = (access_flag)read_u2(content, length, idx);
	method->name_index		 = read_u2(content, length, idx);
	method->descriptor_index = read_u2(content, length, idx);
	method->attributes_count = read_u2(content, length, idx);
	method->attributes = (attribute_info *)safemalloc(method->attributes_count *
													  sizeof(attribute_info));

	for (u2 i = 0; i < method->attributes_count; i++) {
		read_attribute_info(content, length, idx, &method->attributes[i]);
	}
}

/*
Parses the 'content' array (containing the contents of a class file)
and returns an allocated and initialized class_file struct.
*/
static class_file *parse_class_file(const u1 *content, const uint32_t length) {
	if (content == NULL || length <= 0) {
		return NULL;
	}

	uint32_t idx = 0;

	const u4 magic_number  = read_u4(content, length, &idx);
	const u2 minor_version = read_u2(content, length, &idx);
	const u2 major_version = read_u2(content, length, &idx);
	printf("Magic : %d (0x%08X)\n", magic_number, magic_number);
	printf("Minor : %d (0x%04X)\n", minor_version, minor_version);
	printf("Major : %d (0x%04X)\n", major_version, major_version);

	const u2 constant_pool_count = read_u2(content, length, &idx);

	cp_info *constant_pool =
		(cp_info *)safemalloc((constant_pool_count - 1) * sizeof(cp_info));

	printf("\nConstant pool\n");
	read_constant_pool(content, length, &idx, constant_pool,
					   constant_pool_count);
	printf("\n");

	const u2 access_flags = read_u2(content, length, &idx);
	printf("Access flags : 0x%04X", access_flags);
	for (u2 i = 0; i < 8 * sizeof(u2); i++) {
		if (access_flags & (1 << i)) {
			printf(" %s", get_class_access_flag_name(access_flags & (1 << i)));
		}
	}
	printf("\n");

	const u2 this_class = read_u2(content, length, &idx);
	printf("this_class  : #%d\n", this_class);

	const u2 super_class = read_u2(content, length, &idx);
	printf("super_class : #%d\n", super_class);

	const u2 interfaces_count = read_u2(content, length, &idx);
	u2 *interfaces			  = (u2 *)safemalloc(interfaces_count * sizeof(u2));
	printf("\n%d interfaces\n", interfaces_count);
	printf("Interfaces\n");
	for (u2 i = 0; i < interfaces_count; i++) {
		interfaces[i] = read_u2(content, length, &idx);
		printf(" %d : #%d\n", i, interfaces[i]);
	}
	printf("\n");

	const u2 fields_count = read_u2(content, length, &idx);
	field_info *fields =
		(field_info *)safemalloc(fields_count * sizeof(field_info));
	printf("%d fields\n", fields_count);
	printf("Fields\n");
	for (u2 i = 0; i < fields_count; i++) {
		printf(" %d : ", i);
		read_field_info(content, length, &idx, &fields[i]);
		printf("\n");
	}
	printf("\n");

	const u2 methods_count = read_u2(content, length, &idx);
	method_info *methods =
		(method_info *)safemalloc(methods_count * sizeof(method_info));
	printf("%d methods\n", methods_count);
	printf("Methods\n");
	for (u2 i = 0; i < methods_count; i++) {
		read_method_info(content, length, &idx, &methods[i]);
	}
	printf("\n");

	const u2 attributes_count = read_u2(content, length, &idx);
	attribute_info *attributes =
		(attribute_info *)safemalloc(attributes_count * sizeof(attribute_info));
	printf("%d attributes\n", attributes_count);
	printf("Attributes\n");
	for (u2 i = 0; i < attributes_count; i++) {
		read_attribute_info(content, length, &idx, &attributes[i]);
	}
	printf("\n");

	class_file *cf = (class_file *)safemalloc(sizeof(class_file));

	cf->magic_number		= magic_number;
	cf->minor_version		= minor_version;
	cf->major_version		= major_version;
	cf->constant_pool_count = constant_pool_count;
	cf->constant_pool		= constant_pool;
	cf->access_flags		= (access_flag)access_flags;
	cf->this_class			= this_class;
	cf->super_class			= super_class;
	cf->interfaces_count	= interfaces_count;
	cf->interfaces			= interfaces;
	cf->fields_count		= fields_count;
	cf->fields				= fields;
	cf->methods_count		= methods_count;
	cf->methods				= methods;
	cf->attributes_count	= attributes_count;
	cf->attributes			= attributes;

	return cf;
}

/*
Takes as input the name of the .class file to read.
Reads the .class file fully, if successfull, and returns a
pointer to a class_file structure allocated dinamically.
This class_file needs to be deallocated with class_file_destroy().
*/
class_file *read_class_file(const char *filename) {
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
void class_file_destroy(class_file *cf) {
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