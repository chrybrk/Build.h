#define BUILD_IMPLEMENTATION
#include "../../build.h"

// This example is shit, i just did it to try it out.

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

char *filter_h(char *file)
{
  if (file[strlen(file) - 1] == 'c')
    return file;

  return NULL;
}

char *map_source(char *source)
{
  return bh_fmt("%s.o", source);
}

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  bh_mkdir("bin");

  bh_files_t files = { 0 };
  assert(bh_files_get("test/", &files));

  bh_files_t source = { 0 };
  bh_filter(&files, &source, filter_h);

  bh_async_t async = { 0 };
  bh_foreach(&source, file, {
    bh_push_async(&async, bh_fmt("gcc -o %s.o -c %s", file, file));
  });

  if (bh_execute_async(&async)) {
    bh_files_t objects = { 0 };
    bh_map(&source, &objects, map_source);

    char *object_string = bh_files_to_string(&objects, ' ');
    bh_execute(bh_fmt("gcc -o bin/main %s", object_string));
  }

  bh_arena_free(&arena);

  return 0;
}
