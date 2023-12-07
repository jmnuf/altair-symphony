#include <stdbool.h>

#define ALTAIR_ORCHESTRATE
#include "../src/altair-symphony.h"

int main(int argc, char *argv[]) {
	INFO("Started script!\n");
	INFO("Built using c compiler: %s\n", compiler);

	// Build Order
	SwordOrder order = sword_order_new();
	INFO("Initialized an order\n");

	bool check_rebuild = true;
	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			if (strcmp(argv[i], "-nrb") == 0) {
				check_rebuild = false;
				INFO("Ignoring rebuild: `-nrb` flag\n");
				break;
			}
		}
	}

	if (check_rebuild) {
		char* input_files[2] = { __FILE__, "./src/altair-symphony.h" };
		INFO("Checking files for rebuilding:\n\t- %s\n\t- %s\n", input_files[0], input_files[1]);
		char* binary_file = argv[0];
		int should_rebuild = needs_rebuild(binary_file, 2, input_files);
		if (should_rebuild < 0) {
			ERROR("Failed to check for rebuilding because of error, skipping...\n");
		} else if (should_rebuild > 0) {
			INFO("Rebuilding foo will be attempted cause of changes in source files...\n");
			string_t n_argv[argc + 1];
			for (int i = 0; i < argc; ++i) {
				n_argv[i] = argv[i];
			}
			n_argv[argc] = "-nrb";
			string_t headers[1] = {
				"./src/altair-symphony.h"
			};
			rebuild_if_needed_source_with_headers(argc, n_argv, __FILE__, 1, headers);
		} else {
			INFO("No rebuild required...\n");
		}
	}

	INFO("Setting up order pieces\n");

	if (!sword_order_append(&order, "echo", "hello", "world!").success) {
		ERROR("Failed to append to order");
		return 1;
	}
	INFO("Set array with %d elements\n", order.pieces_count);

	// Compile Order
	string_t compiled = sword_order_compile(&order);
	INFO("Order has been compiled and ready for execution!\n");

	// Execute Order
	SYS("%s\n", order.compiled);
	int result = system(order.compiled);
	INFO("Command ended with result %d\n", result);

	return 0;
}

