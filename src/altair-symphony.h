
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
	#include <direct.h>
	#define GETCWD _getcwd
#else
	#include <unistd.h>
	#define GETCWD getcwd
#endif

#ifndef __ALTAIR_SYMPHONY__
#define __ALTAIR_SYMPHONY__

#define STRINGIFY(...) #__VA_ARGS__
#define DELAYED_STRINGIFY(...) STRINGIFY(__VA_ARGS__)

#define FPRINT(f, ...) fprintf(f, __VA_ARGS__); fflush(f)
#define INFO(...) fprintf(stdout, "[INFO]: "); FPRINT(stdout, __VA_ARGS__)
#define SYS(...) fprintf(stdout, "[SYS]: "); FPRINT(stdout, __VA_ARGS__)
#define ERROR(...) fprintf(stderr, "[ERROR]: "); FPRINT(stderr, __VA_ARGS__)

#define MTALLOC(T) malloc(sizeof(T))

#ifndef SWORD_ORDER_BASE_CAPACITY
#	define SWORD_ORDER_BASE_CAPACITY 50
#endif

#ifndef COMPILER
#	define COMPILER compiler
#	if defined(__MINGW32__)
// I don't know, this is the ming32 I have ok, cygwin sometimes don't work great but it's ok
const char *compiler = "i686-w64-mingw32-gcc";
#	elif defined(__GNUC__)
// Man's on linux or wsl or using cygwin (what I'm using don't attack me plz)
const char *compiler = "gcc";
#	else
#		undef COMPILER
#	endif
#endif

typedef char* string_t;

typedef struct {
	int length;
	int capacity;
	string_t *items;
} StringList;

typedef struct {
	int length;
	int capacity;
	void **items;
} VoidList;

#define list_append_va(T, l, ...) \
	do { \
		int len = (l)->length, cap = (l)->capacity;\
		va_list args;\
		va_start(args, l);\
		T item;\
		while ((item = va_args(args, T)) != NULL) {\
			if (len == cap) {\
				cap += 10;\
				void **new_ptr = realloc((l)->items, sizeof(T) * cap);\
				if (new_ptr == NULL) {\
					ERROR("Failed to allocate more memory for list");\
					exit(1);\
				}\
				(l)->items = new_ptr;\
				(l)->capacity = cap;\
			}\
			(l)->items[len++] = item;\
		}\
		(l)->length = len;\
	} while(0)
#define list_append(T, l, ...) list_append_va(T, l, __VA_ARGS__, NULL)

#define str_list_append(l, ...) list_append(char*, l, __VA_ARGS__)

typedef struct {
	string_t *pieces;
	int pieces_count;
	int capacity;
	string_t compiled;
} SwordOrder;

typedef struct {
	int pieces_added;
	int success;
} SwordOrderAppendResult;

SwordOrder sword_order_new_with_capacity(int capacity);
SwordOrder sword_order_new();

SwordOrderAppendResult sword_order_append_va(SwordOrder *order, ...);
#define sword_order_append(order, ...) sword_order_append_va(order, __VA_ARGS__, NULL)

string_t sword_order_materialize(SwordOrder *order);
#define sword_order_compile(order) sword_order_materialize(order)
int sword_order_shoot(SwordOrder *order);
#define sword_order_execute(order) sword_order_shoot(order)
void sword_order_dematerialize(SwordOrder *order);
#define sword_order_clear(order) sword_order_dematerialize(order)
int sword_order_materialize_shoot_clean(SwordOrder *order);

string_t str_create_va(string_t first, ...);
#define str_create(...) str_create_va(__VA_ARGS__, NULL)
string_t str_arr_flatten(string_t array[], int length);
string_t str_arr_join(string_t array[], int length, string_t joiner);
int str_ends_with(string_t str, string_t suffix);

int needs_rebuild(string_t output_file, int sources_count, string_t *source_files);
void rebuild_if_needed_sources(int argc, string_t argv[], int sources_c, string_t sources_v[]);
void rebuild_if_needed_source_with_headers(int argc, string_t argv[], string_t source_file, int headers_c, string_t headers_v[]);
void rebuild_if_needed_sources_with_headers(int argc, string_t argv[], int sources_c, string_t sources_v[], int headers_c, string_t headers_v[]);

// NOTE: If you're using spaces in your directory or file name this will fail every time! This is a feature! Stop using spaces bozo
#define REBUILD_SELF(argc, argv) \
	do {\
		string_t binary_path = argv[0]; \
		string_t main_filev[1] = { __FILE__ }; \
		int rebuild_main_needed = needs_rebuild(binary_path, 1, main_filev); \
		if (rebuild_main_needed < 0) { \
			INFO("Skipping rebuild check cause of error...\n"); \
			break; \
		} else if (rebuild_main_needed == 0) { \
			break; \
		} \
		INFO("Rebuilding self...\n"); \
		SwordOrder order = sword_order_new(); \
		string_t old_bin_path; \
		/* TODO: Move this exe check to a system macro check to lower outputted code on expansion */ \
		if (str_ends_with(binary_path, ".exe")) { \
			const char *exe_ext = ".exe";\
			int exe_len = strlen(exe_ext);\
			int bin_len = strlen(binary_path);\
			const char *suffix = ".old";\
			int suffix_len = strlen(suffix);\
			/* Length of bin path + length of ".old" + NULL terminator */ \
			int old_bin_len = bin_len + suffix_len + 1;\
			char old_bin[substr_len];\
			int idx = 0;\
			for (int i = 0; i < bin_len - exe_ext_len; i++) {\
				old_bin[idx++] = binary_path[i];\
			}\
			for (int i = 0; i < suffix_len; i++) {\
				old_bin[idx++] = suffix[i];\
			}\
			for (int i = 0; i < exe_ext_len; i++) {\
				old_bin[idx++] = exe_ext[i];\
			}\
			old_bin[idx] = '\0';\
			old_bin_path = old_bin; \
			/* We assume windows so we use window's rename, iladies */\
			sword_order_append(&order, "ren");\
		} else { \
			const char *suffix = ".old";\
			int suffix_len = strlen(suffix);\
			int new_len = strlen(binary_path) + suffix_len + 1;\
			char old_bin[new_len];\
			sprintf(old_bin, "%s%s", binary_path, suffix);\
			old_bin[new_len - 1] = '\0';\
			old_bin_path = old_bin; \
			/* We assume non-gamer so we use mv to move to rename */\
			sword_order_append(&order, "mv", "-v");\
		} \
		sword_order_append(&order, binary_path, old_bin_path); \
		sword_order_materialize_shoot_clean(&order); \
		\
		if (strlen(binary_path) < 2 || binary_path[0] != '.' || binary_path[1] != '/') {\
			binary_path = str_create("./", binary_path);\
		} else {\
			binary_path = str_create(binary_path);\
		}\
	 \
		sword_order_append(&order, compiler, __FILE__, "-o", binary_path); \
		int result = sword_order_materialize_shoot_clean(&order); \
		\
		if (result != 0) { \
			ERROR("Failed to rebuild...\n"); \
			free(binary_path);\
			break; \
		} \
	 \
		sword_order_append(&order, binary_path);\
		if (argc > 1) for (int i = 1; i < argc; ++i) { \
			sword_order_append(&order, argv[i]); \
		} \
		result = sword_order_materialize_shoot_clean(&order); \
		free(binary_path);\
		exit(result); \
	} while(0)


int create_dir_if_missing(string_t dir_path);
int delete_dir_if_exists(string_t dir_path);

int create_file_if_missing(string_t file_path);
int delete_file_if_exists(string_t file_path);

#ifdef ALTAIR_ORCHESTRATE

SwordOrder sword_order_new() {
	return sword_order_new_with_capacity(SWORD_ORDER_BASE_CAPACITY);
}
SwordOrder sword_order_new_with_capacity(int starting_capacity) {
	SwordOrder order = {0};

	order.pieces_count = 0;
	order.capacity = starting_capacity;
	order.pieces = malloc(sizeof(char[starting_capacity]));
	order.compiled = NULL;

	return order;
}

/**
 * @param {SwordOrder*} order pointer to append to
 * @param {...char*} variant arguments list of strings to append to the command
 * @returns {SwordOrderAppendResult} object containing how many items were added and if it was succesful
 */
SwordOrderAppendResult sword_order_append_va(SwordOrder *order, ...) {
	va_list pieces;
	va_start(pieces, order);

	string_t new_piece;
	SwordOrderAppendResult result = {
		.pieces_added = 0,
		.success = 1,
	};

	int capacity = order->capacity;

	while((new_piece = va_arg(pieces, string_t)) != NULL) {
		int new_size = order->pieces_count + 1;

		if (new_size >= capacity) {
			capacity = capacity + 10;
			string_t* new_ptr = realloc(order->pieces, capacity * sizeof(string_t));
			if (new_ptr == NULL) {
				ERROR("Failed to allocate more memory for order pieces\n");
				va_end(pieces);
				result.success = 0;
				return result;
			}
			order->pieces = new_ptr;
			order->capacity = capacity;
		}
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
string_t sword_order_materialize(SwordOrder *order) {
	string_t compiled = str_arr_join(order->pieces, order->pieces_count, " ");
	order->compiled = compiled;
	if (compiled == NULL) {
		ERROR("Not enough faith and views to materialize this chorus of guns and swords\n\tCouldn't allocate memory when compiled SwordOrder\n");
	}
	return compiled;
}

/**
 * Make the swords and guns work
 * Execute compiled order using system
 *
 * @param {SwordOrder*} order to execute
 * @returns {int} the result of the execution or -1 when the order isn't compiled
 */
int sword_order_shoot(SwordOrder *order) {
	if (order->compiled == NULL) {
		ERROR("Swords and guns haven't been materialized\n\tSwordOrder command hasn't been compiled. Remember to call sword_order_materialize\n");
		return -1;
	}
	char* command = order->compiled;
	SYS("%s\n", command);
	int result = system(command);
	return result;
}

/**
 * Execute compiled order using system
 * Make the swords and guns advance
 *
 * @param {SwordOrder*} order to execute
 * @returns {int} the result of the execution or -1 when the order isn't compiled
 */
void sword_order_dematerialize(SwordOrder *order) {
	if (order->compiled != NULL) {
		free(order->compiled);
		order->compiled = NULL;
	}
	order->pieces_count = 0;
}

int sword_order_materialize_shoot_clean(SwordOrder *order) {
	if (sword_order_materialize(order) == NULL) {
		return -1;
	}
	int result = sword_order_shoot(order);
	sword_order_dematerialize(order);
	return result;
}

string_t str_create_va(string_t first, ...) {
	va_list pieces;
	va_start(pieces, first);
	string_t new_piece;

	// TODO: Create a string list type for this case
	SwordOrder order = sword_order_new_with_capacity(50);
	sword_order_append(&order, first);

	while((new_piece = va_arg(pieces, string_t)) != NULL) {
		sword_order_append(&order, new_piece);
	}
	va_end(pieces);
	string_t new_str = str_arr_flatten(order.pieces, order.pieces_count);
	return new_str;
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
			INFO("Output file not found. Build needed assumed\n");
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

void rebuild_if_needed_source_with_headers(int argc, string_t argv[], string_t source_file, int headers_c, string_t headers_v[]) {
	string_t binary_path = argv[0];
	SwordOrder order = sword_order_new_with_capacity(25);
	sword_order_append(&order, source_file);
	for (int i = 0; i < headers_c; ++i) {
		sword_order_append(&order, headers_v[i]);
	}

#if defined(_WIN32) || defined(_WIN64)
	if (strlen(binary_path) < 2 || binary_path[0] != '.' || binary_path[1] != '\\') {
		if (binary_path[1] == '/') {
			char bin_substr[strlen(binary_path) - 2 + 1];
			for (int i = 2; i < strlen(binary_path); ++i) {
				bin_substr[i - 2] = binary_path[i];
			}
			binary_path = str_create(".\\", bin_substr, "");
		} else {
			binary_path = str_create(".\\", binary_path, "");
		}
#else
	if (strlen(binary_path) < 2 || binary_path[0] != '.' || binary_path[1] != '/') {
		binary_path = str_create("./", binary_path, "");
#endif
	} else {
		binary_path = str_create(binary_path);
	}
#if defined(_WIN32) || defined(_WIN64)
	{
		// Idk what is going on, windows being windows, but sometimes the .exe matters, sometimes it doesn't, I forgot how I setup me windows ごめんね
		if (!str_ends_with(binary_path, ".exe")) {
			int bin_path_len = strlen(binary_path);
			const string_t exe_ext = ".exe";
			int exe_ext_len = strlen(exe_ext);
			int new_len = bin_path_len + exe_ext_len;
			char *new_ptr = realloc(binary_path, new_len + 1);
			if (new_ptr == NULL) {
				ERROR("Failed to allocate memory\n");
				exit(1);
			}
			binary_path = new_ptr;
			for(int i = 0; i < exe_ext_len; ++i) {
				binary_path[i + bin_path_len] = exe_ext[i];
			}
			binary_path[new_len] = '\0';
		}
	}
#endif
	
	int should_rebuild = needs_rebuild(binary_path, order.pieces_count, order.pieces);
	// On error or no rebuild needed exit
	if (should_rebuild <= 0) {
		return;
	}

	sword_order_clear(&order);
	string_t old_bin_path;
#if defined(_WIN32) || defined(_WIN64)
	if (str_ends_with(binary_path, ".exe")) {
		const string_t suffix = ".old";
		int suffix_len = strlen(suffix);
		const string_t exe_ext = ".exe";
		int exe_ext_len = strlen(exe_ext);
		int old_bin_len = strlen(binary_path) + suffix_len;
		old_bin_path = malloc(old_bin_len + 1);
		if (old_bin_path == NULL) {
			ERROR("Faield to allocate memory");
			exit(1);
		}
		int idx = 0;
		for (int i = 0; i < strlen(binary_path) - exe_ext_len; ++i) {
			old_bin_path[idx++] = binary_path[i];
		}
		for (int i = 0; i < suffix_len; ++i) {
			old_bin_path[idx++] = suffix[i];
		}
		for (int i = 0; i < exe_ext_len; ++i) {
			old_bin_path[idx++] = exe_ext[i];
		}
		old_bin_path[old_bin_len] = '\0';
	}
#else
	// TODO: Don't malloc for this operation, I just did it initially cause I didn't know better and bing chat generated this function LUL
	old_bin_path = str_create(binary_path, ".old");
#endif
	
	if (rename(binary_path, old_bin_path) == 0) {
		SYS("Renamed '%s' -> '%s'\n", binary_path, old_bin_path);
	} else {
		ERROR("Failed renaming '%s' to '%s'\n\tDoing dangerous rebuild-relaunch attempt...\n", binary_path, old_bin_path);
	}

	free(old_bin_path);

	// TODO: Don't header files sometimes require to be referenced by compiler like a lib thingy? IDK, IDK C bro
	sword_order_append(&order, compiler, source_file, "-o", binary_path);
	int result = sword_order_materialize_shoot_clean(&order);
	if (result != 0) {
		ERROR("Failed to rebuild");
		free(binary_path);
		return;
	}
	{
		char *tmp = binary_path;
		binary_path = str_create("\"", tmp, "\"");
		free(tmp);
	}
	sword_order_append(&order, binary_path);
	for (int i = 1; i < argc; ++i) {
		sword_order_append(&order, argv[i]);
	}
	result = sword_order_materialize_shoot_clean(&order);
	free(binary_path);
	exit(result);
}

void rebuild_if_needed_sources(int argc, string_t argv[], int sources_c, string_t sources_v[]) {
	string_t binary_path = argv[0];
	int rebuild_sources_needed = needs_rebuild(binary_path, sources_c, sources_v);
	if (rebuild_sources_needed < 0) {
		INFO("Skipping rebuild check cause of error...\n");
		return;
	} else if (rebuild_sources_needed == 0) {
		return;
	}
	INFO("Rebuilding self...\n");
	SwordOrder order = sword_order_new();
	// Move current binary to avoid issues
	string_t old_bin_path;
	if (str_ends_with(binary_path, ".exe")) {
		old_bin_path = str_create(binary_path, ".old.exe");
	} else {
		old_bin_path = str_create(binary_path, ".old");
	}
	sword_order_append(&order, "mv", binary_path, old_bin_path);
	sword_order_materialize_shoot_clean(&order);
	free(old_bin_path);

	sword_order_append(&order, compiler);

	for (int i = 0; i < sources_c; ++i) {
		sword_order_append(&order, sources_v[i]);
	}

	sword_order_append(&order, "-o", binary_path);

	sword_order_compile(&order);

	SYS("%s\n", order.compiled);
	int result = system(order.compiled);

	if (result != 0) {
		ERROR("Failed to rebuild\n");
		return;
	}

	free(order.pieces);
	order.pieces_count = 0;
	free(order.compiled);

	INFO("%s has been rebuilt. Relaunching...\n", argv[0]);
	
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
