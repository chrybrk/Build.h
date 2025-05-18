#define BUILD_IMPLEMENTATION
#include "build.h"

const char *build_bin = "build";
const char *build_source = "build.c";
extern bh_arena_t *build_arena;

bh_arena_t arena = { 0 };

int main(void)
{
  bh_init_arena(&arena, 1024 * 1024 * 16);
  build_arena = &arena;

  return 0;
}
