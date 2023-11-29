#define ALTAIR_ORCHESTRATE
#include "../src/altair-symphony.h"

int rebuild_self() {
	SwordOrder order = sword_order_new();
	char *cmd = "gcc ./example/foo.c -o ./foo";
	SYS("%s\n", cmd);
	int result = system(cmd);
	if (result != 0) {
		return 1;
	}
#ifdef _WIN32
	cmd = ".\\foo.exe";
#else
	cmd = "./foo";
#endif
	SYS("%s\n", cmd);
	result = system(cmd);
	return result;
}

int main(int argc, char *argv[]) {

	INFO("Started script!\n");
	// Build Order
	SwordOrder order = sword_order_new();
	INFO("Initialized an order\n");

	INFO("Checking if foo requires a rebuild\n");
	char* input_files[1] = { "./example/foo.c" };
	char* binary_file = argv[0];
	int should_rebuild = needs_rebuild(binary_file, 1, input_files);
	if (should_rebuild < 0) {
		ERROR("Failed to check for rebuilding because of error, skipping...\n");
	} else if (should_rebuild > 0) {
		INFO("Rebuilding foo is needed, proceeding...\n");
	} else {
		INFO("No rebuild required...\n");
	}
	rebuild_self_if_needed(1, (char*[]) { "./example/foo.c" }, argc, argv);

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

