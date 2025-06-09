#define BUILD_IMPLEMENTATION
#include "build.h"

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  bh_files_t files = { 0 };
  assert(bh_recursive_files_get(".", &files));
  bh_foreach(&files, file, printf("%s\n", file));

  bh_arena_free(&arena);

  return 0;
}
