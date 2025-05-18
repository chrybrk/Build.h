#define BUILD_IMPLEMENTATION
#include "../build.h"

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

int square(int x)
{
  return x * x;
}

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  bh_define_darray(int) darray_int_t;

  darray_int_t array = { 0 };
  for (int i = 0; i < 10; ++i) {
    bh_darray_push(&array, i);
  }

  darray_int_t out = { 0 };
  bh_map(&array, &out, square);

  bh_foreach(&out, item, printf("%d\n", item));

  bh_darray_free(&out);
  bh_darray_free(&array);
  bh_arena_free(&arena);

  return 0;
}
