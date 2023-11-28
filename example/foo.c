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
#ifdef _WIN32
	int should_rebuild = needs_rebuild("./example/foo.c", "./foo.exe");
#else
	int should_rebuild = needs_rebuild("./example/foo.c", "./foo");
#endif
	if (should_rebuild < 0) {
		ERROR("Failed to check for rebuilding because of error, skipping...\n");
	} else if (should_rebuild > 0) {
		INFO("Rebuilding foo is needed, proceeding...\n");
		return rebuild_self();
	} else {
		INFO("No rebuild required...\n");
	}

	INFO("Setting up order pieces\n");
	if (!SWORD_ORDER_APPEND(&order, "echo", "hello", "world!").success) {
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

