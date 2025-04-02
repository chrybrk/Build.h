# Build.h

`build.h` is a single-header library that acts as a build system, it works more-or-less like shell-script, with many built-in features:

- `Execute`
- `INFO`
- `WARN`
- `ERROR`
- `formate_string`
- `sub-string`
- `join-string`
- `replace`
- `conversion between array and string`
- `execute shell commands`
- `check for directory or files, or create one`
- `read or write to file`
- `generic dynamic array`
- `generic ordered hashmap`


# Start here

```c 
#define IMPLEMENT_BUILD_H
#define BUILD_ITSELF
#include "build.h"

const char *build_bin = "build";
const char *build_source = "build.c";

int main(int argc, char **argv)
{
	INFO("Hello, world!");

	return 0;
}
```

## Build
```txt
$ cc -o build build.c
$ ./build
```

After that it will re-build itself, when it will detect changes.
