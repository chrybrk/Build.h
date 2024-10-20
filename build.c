#define IMPLEMENT_BUILD_C

#include "build.h"

#define CC "gcc"
#define CFALGS "-O2", "-g0", "-static"

int main(int argc, char **argv)
{
	CMD(CC, CFALGS, "-Iexample/", "example/hello_world.c", "-o", "out");

	return 0;
}
