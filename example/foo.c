#define ALTAIR_ORCHESTRATE
#include "../src/altair-symphony.h"

int main(int argc, char *argv[]) {
	INFO("Started script!\n");
	// Build Order
	SwordOrder order = sword_order_new();
	INFO("Initialized an order\n");

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

