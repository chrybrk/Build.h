# Build.h

`build.h` is a single-header library that acts as a build system, it works more-or-less like shell-script, with many built-in features:

# Start here

## Example
```c 
#define BUILD_IMPLEMENTATION
#include "build.h"

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  bh_log(1, "Hello, World!\n");

  bh_arena_free(&arena);

  return 0;
}
```

## Bootstraping

```txt
$ cc -o build build.c
$ ./build
```
