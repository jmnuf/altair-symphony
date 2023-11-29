#ifndef __ALTAIR_SYMPHONY__
#define __ALTAIR_SYMPHONY__
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
	#include <direct.h>
	#define GETCWD _getcwd
#else
	#include <unistd.h>
	#define GETCWD getcwd
#endif
#define STRINGIFY(...) #__VA_ARGS__
#define DELAYED_STRINGIFY(...) STRINGIFY(__VA_ARGS__)

#define FPRINT(f, ...) fprintf(f, __VA_ARGS__); fflush(f)
#define INFO(...) fprintf(stdout, "[INFO]: "); FPRINT(stdout, __VA_ARGS__)
#define SYS(...) fprintf(stdout, "[SYS]: "); FPRINT(stdout, __VA_ARGS__)
#define ERROR(...) fprintf(stderr, "[INFO]: "); FPRINT(stderr, __VA_ARGS__)

#define MTALLOC(T) malloc(sizeof(T))

typedef char* string_t;

typedef struct {
	string_t *pieces;
	int pieces_count;
	string_t compiled;
} SwordOrder;

typedef struct {
	int pieces_added;
	int success;
} SwordOrderAppendResult;

SwordOrder sword_order_new();
SwordOrderAppendResult sword_order_append_va(SwordOrder *order, ...);
#define sword_order_append(order, ...) sword_order_append_va(order, __VA_ARGS__, NULL)
string_t sword_order_compile(SwordOrder *order);

string_t str_arr_flatten(string_t array[], int length);
string_t str_arr_join(string_t array[], int length, string_t joiner);
int str_ends_with(string_t str, string_t suffix);

int needs_rebuild(string_t output_file, int sources_count, string_t *source_files);
void rebuild_self_if_needed(int input_files_c, string_t input_files_v[], int argc, string_t argv[]);

int create_dir_if_missing(string_t dir_path);
int delete_dir_if_exists(string_t dir_path);

int create_file_if_missing(string_t file_path);
int delete_file_if_exists(string_t file_path);

#ifdef ALTAIR_ORCHESTRATE
#include <stdarg.h>
#include <errno.h>

SwordOrder sword_order_new() {
	SwordOrder order = {0};

	order.pieces = NULL;
	order.pieces_count = 0;
	order.compiled = NULL;

	return order;
}

SwordOrderAppendResult sword_order_append_va(SwordOrder *order, ...) {
	va_list pieces;
	va_start(pieces, order);

	string_t new_piece;
	SwordOrderAppendResult result = {
		.pieces_added = 0,
		.success = 1,
	};

	while((new_piece = va_arg(pieces, string_t)) != NULL) {
		int new_size = order->pieces_count + 1;
		string_t* new_ptr = realloc(order->pieces, new_size * sizeof(string_t));
		if (new_ptr == NULL) {
			va_end(pieces);
			result.success = 0;
			return result;
		}

		order->pieces = new_ptr;
		order->pieces[order->pieces_count++] = new_piece;
	}
	va_end(pieces);
	return result;
}

/**
 * Compiles the order into a single string for execution
 *
 * @param {SwordOrder*} order to compile
 * @returns {char*} the compiled code
 */
string_t sword_order_compile(SwordOrder *order) {
	string_t compiled = str_arr_join(order->pieces, order->pieces_count, " ");
	order->compiled = compiled;
	return compiled;
}

/**
 * Flatten array to a single string with nothing between strings
 *
 * @param {char**} array of strings
 * @param {int} length of array
 * @returns {char*} A new string which requires to be freed
 */
string_t str_arr_flatten(string_t arr[], int len) {
	if (len < 1) {
		return NULL;
	}
	string_t flat;
	if (len == 1) {
		int size = strlen(arr[0]) + 1;
		flat = malloc(size);
		strcpy(flat, arr[0]);
		return flat;
	}
	int size = 0, off = 0;
	for (int i = 0; i < len; ++i) {
		size += strlen(arr[i]);
	}
	flat = malloc(size + 1); // Add one for the null terminator
	for (int i = 0; i < len; ++i) {
		off += sprintf(flat + off, "%s", arr[i]);
	}
	return flat;
}

/**
 * Flatten array to a single string with a joiner string between each string
 * If joiner is set to NULL defaults to a space
 *
 * @param {char *} array of strings
 * @param {int} length of array
 * @param {char*} string to use between strings
 * @returns {char*} A new string which requires to be freed
 */
string_t str_arr_join(string_t arr[], int len, string_t joiner) {
	if (len < 1) {
		return NULL;
	}
	string_t flat;
	if (len == 1) {
		int size = strlen(arr[0]) + 1;
		flat = malloc(size);
		strcpy(flat, arr[0]);
		return flat;
	}
	if (joiner == NULL) {
		joiner = " ";
	}
	int joiner_len = strlen(joiner);
	int size = 0, off = 0;
	for (int i = 0; i < len; ++i) {
		size += strlen(arr[i]);
		if (i + 1 < len) {
			size += joiner_len;
		}
	}
	flat = malloc(size + 1); // Add one for the null terminator
	for (int i = 0; i < len; ++i) {
		if (i + 1 < len) {
			off += sprintf(flat + off, "%s%s", arr[i], joiner);
		} else {
			off += sprintf(flat + off, "%s", arr[i]);
		}
	}
	return flat;
}

/**
 * Check if a string ends with a certain suffix
 *
 * @param {char*} string to check for suffix
 * @param {char*} suffix substring to look for
 * @returns {int} 0(false) if suffix not found and 1(true) if suffix found
 */
int str_ends_with(string_t str, char *suffix) {
	size_t str_len = strlen(str);
	size_t suf_len = strlen(suffix);
	if (str_len < suf_len) {
		return 0;
	}
	return strncmp(str + str_len - suf_len, suffix, suf_len) == 0;
}

/**
 * Check if a program needs to be rebuilt/recompiled based on a source file.
 * This can be used for that amazing ability to build C programs using a C program!
 * This code is definitely not mine, not sure where I took it from anymore tho
 *
 * @param {char*} output file path
 * @param {int} source files count
 * @param {char**} source files path
 * @returns {int} 1 if build needed, 0 if no, -1 if error has occured
 */
int needs_rebuild(string_t output_file, int sources_count, string_t *source_files) {
	struct stat statbuf = {0};
	if (stat(output_file, &statbuf) < 0) {
		if (errno == ENOENT) {
			INFO("Output file not found. Build needed assumed");
			return 1;
		}
		ERROR("Failed to stat output file: %s\n", output_file);
		return -1;
	}
	int output_file_time = statbuf.st_mtime;

	for (int i = 0; i < sources_count; ++i) {
		string_t source_file = source_files[i];
		if (stat(source_file, &statbuf) < 0) {
			if (errno == ENOENT) {
				ERROR("Missing source file: %s\n", source_file);
			} else {
				ERROR("Failed to stat source file: %s\n", source_file);
			}
			return -1;
		}

		int source_file_time = statbuf.st_mtime;
		if (source_file_time > output_file_time) {
			INFO("Source file has changed: %s\n", source_file);
			return 1;
		}
	}

	return 0;
}

void rebuild_self_if_needed(int input_files_c, string_t input_files_v[], int argc, string_t argv[]) {
	string_t binary_path = argv[0];
	int rebuild_needed = needs_rebuild(binary_path, input_files_c, input_files_v);
	if (rebuild_needed < 0) {
		INFO("Skipping rebuild check cause of error...");
		return;
	} else if (rebuild_needed == 0) {
		return;
	}
	INFO("Rebuilding self...");
	SwordOrder order = sword_order_new();
	sword_order_append(&order, "gcc");

	for (int i = 0; i < input_files_c; ++i) {
		sword_order_append(&order, input_files_v[i]);
	}

	sword_order_append(&order, "-o", binary_path);

	sword_order_compile(&order);

	SYS("%s\n", order.compiled);
	int result = system(order.compiled);

	if (result != 0) {
		ERROR("Failed to rebuild");
		return;
	}

	free(order.pieces);
	order.pieces_count = 0;
	free(order.compiled);

	INFO("Self has been rebuilt. Relaunching...");
	
	for (int i = 0; i < argc; ++i) {
		sword_order_append(&order, argv[i]);
	}

	sword_order_compile(&order);
	result = system(order.compiled);

	exit(result);
}

/**
 * If directory doesn't exists, it attempts to create it.
 * Does nothing if the directory exists.
 *
 * @param {char*} path of directory
 * @returns {int} result code of mkdir system command
 */
int create_dir_if_missing(string_t dir_path) {
	struct stat st = {0};
	if (stat(dir_path, &st) != -1) {
		return 0;
	}
#define MKDIR_ARR_LEN 3
	char *mkdir_arr[MKDIR_ARR_LEN] = {
		"mkdir \"",
		dir_path,
		"\""
	};
	char *mkdir_cmd = str_arr_flatten(mkdir_arr, MKDIR_ARR_LEN);
#undef MKDIR_ARR_LEN
	SYS("%s\n", mkdir_cmd);
	int result = system(mkdir_cmd);
	free(mkdir_cmd);
	return result;
}

/**
 * If the directory exists, it attempts to delete it.
 * Does nothing if the directory doesn't exist
 * 
 * @param {char*} path of directory
 * @param {int} result code of rm system command
 */
int delete_dir_if_exists(string_t dir_path) {
	struct stat st = {0};
	if (stat(dir_path, &st) == -1) {
		return 0;
	}
#define RM_ARR_LEN 3
	char *arr[RM_ARR_LEN] = {
		"rm -rf \"",
		dir_path,
		"\""
	};
	char *cmd = str_arr_flatten(arr, RM_ARR_LEN);
#undef RM_ARR_LEN
	SYS("%s\n", cmd);
	int result = system(cmd);
	free(cmd);
	return result;
}

/**
 * If the file doesn't exist it attempts to create it,
 * does nothing if it does exist
 *
 * @param {char*} path for file to create
 */
int create_file_if_missing(string_t file_path) {
	FILE *f = fopen(file_path, "r");
	if (f != NULL) {
		fclose(f);
		return 0;
	}
	SYS("touch \"%s\"\n", file_path);
	f = fopen(file_path, "w");
	if (f == NULL) {
		return -1;
	}
	fclose(f);
	return 0;
}

/**
 * If the directory doesn't exist it attempts to create it.
 * Does nothing if it does exist
 *
 * @param {char*} path for directory to create
 * @returns {int} some result code surely
 */
int delete_file_if_exists(string_t file_path) {
}

#endif

#endif
