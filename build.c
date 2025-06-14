#define BUILD_IMPLEMENTATION
#define BUILD_EXECUTE_LOG
#include "build.h"

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  const char *filename = "build.c";

  // it works like any other try-catch
  bh_try {
    // do something ... 
    char *file = bh_file_read(filename);
    bh_uthrow(file, bh_FileNotFoundError);

    // if has no error then this line will execute.
    printf("`%s` found.\n", filename);

  } bh_catch(bh_FileNotFoundError) {

    // else this will execute...
    printf("`%s` not found.\n", filename);
  }

  bh_arena_free(&arena);

  return 0;
}
