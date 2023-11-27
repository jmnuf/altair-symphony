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
SwordOrderAppendResult sword_order_append(SwordOrder *order, ...);
#define SWORD_ORDER_APPEND(order, ...) sword_order_append(order, __VA_ARGS__, NULL)
string_t sword_order_compile(SwordOrder *order);

string_t str_arr_flatten(string_t array[], int length);
string_t str_arr_join(string_t array[], int length, string_t joiner);
int str_ends_with(string_t str, string_t suffix);
int needs_rebuild(string_t base_file, string_t output_file);

int create_dir_if_missing(string_t dir_path);
int delete_dir_if_exists(string_t dir_path);

int create_file_if_missing(string_t file_path);
int delete_file_if_exists(string_t file_path);

#ifdef ALTAIR_ORCHESTRATE
#include <stdarg.h>

SwordOrder sword_order_new() {
	SwordOrder order = {0};

	order.pieces = NULL;
	order.pieces_count = 0;
	order.compiled = NULL;

	return order;
}

SwordOrderAppendResult sword_order_append(SwordOrder *order, ...) {
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
 * @param {char*} array of strings
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
 * @param {char* } string to check for suffix
 * @param {char* } suffix substring to look for
 * @returns {int} 0 if suffix not found and 1 if suffix found
 */
int str_ends_with(string_t str, char *suffix) {
	size_t str_len = strlen(str);
	size_t suf_len = strlen(suffix);
	if (str_len < suf_len) {
		return 0;
	}
	return strncmp(str + str_len - suf_len, suffix, suf_len) == 0;
}

#endif

#endif
