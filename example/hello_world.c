#include "hello_world.h"

size_t print_just_hello_world(const char *msg)
{
	return printf("HELLO WORLD: %s\n", msg);
}

int main(void)
{
	print_just_hello_world("Hey, example!");
	print_just_hello_world("Hey, updated!");

	return 0;
}
