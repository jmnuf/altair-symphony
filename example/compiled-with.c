#define ALTAIR_ORCHESTRATE
#include "../src/altair-symphony.h"

int main(int argc, char *argv[]) {
	string_t n_argv[argc + 1];
	int no_rebuild = 0;
	for (int i = 0; i < argc; ++i) {
		n_argv[i] = argv[i];
		if (strcmp(argv[i], "-nrb") == 0) {
			no_rebuild = 1;
			break;
		}
	}
	if (no_rebuild != 1) {
		n_argv[argc] = "-nrb";
		string_t headers[1] = {"./src/altair-symphony.h"};
		rebuild_if_needed_source_with_headers(argc, n_argv, __FILE__, 1, headers);
	}
	INFO("Hello world!\n");
	INFO("Compiled with compiler: %s\n", compiler);
	return 0;
}

