#define IMPLEMENT_BUILD_C

#include "build.h"

#define CFLAGS "-Wall", "-O3"

int main(int argc, char *argv[])
{
	if (
			needs_recompilation(
				"out", 
				(const char*[]){ "example/hello_world.c", "example/hello_world.h" }, 
				2
			)
	)
	{
		INFO("Building `build` project.");
		CMD("cc", "example/hello_world.c", "-Iexample/", CFLAGS, "-o", "out");
	}
	else
	{
		INFO("Example is up-to-dated.");
	}

	return 0;
}
