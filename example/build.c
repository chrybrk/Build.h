#define BUILD_IMPLEMENTATION
#include "../../build.h"

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

static const char *TARGET_PLATFORM = "PLATFORM_DESKTOP_GLFW";
static const char *GRAPHICS = "GRAPHICS_API_OPENGL_33";
static const char *GLFW_API = "_GLFW_X11";

static bh_strings_t CFLAGS = { 0 };
static bh_strings_t INCLUDES = { 0 };
static bh_strings_t LDFLAGS = { 0 };

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  bh_strings_t LOC_CFLAGS = { 0 };
  assert(bh_string_to_array(
    &LOC_CFLAGS,
    "-Wall -D_GNU_SOURCE -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing",
    ' '
  ));

  bh_darray_push_mul(&CFLAGS, LOC_CFLAGS.buffer, LOC_CFLAGS.index);
  bh_darray_push(&CFLAGS, bh_fmt("-D%s", TARGET_PLATFORM));
  bh_darray_push(&CFLAGS, bh_fmt("-D%s", GRAPHICS));
  bh_darray_push(&CFLAGS, bh_fmt("-D%s", GLFW_API));

  bh_darray_push(&INCLUDES, "-Isrc/external/glfw/include");

  bh_strings_t LOC_LDFLAGS = { 0 };
  assert(bh_string_to_array(&LOC_LDFLAGS, "-lGL -lc -lm -lpthread -ldl -lrt -lm", ' '));
  bh_darray_push_mul(&LDFLAGS, LOC_LDFLAGS.buffer, LOC_LDFLAGS.index);

  // create bin folder
  bh_mkdir("bin");

  bh_files_t raylib_source = { 0 };
  bh_darray_push(&raylib_source, "src/rcore.c");
  bh_darray_push(&raylib_source, "src/rglfw.c");
  bh_darray_push(&raylib_source, "src/rcore.c");
  bh_darray_push(&raylib_source, "src/rshapes.c");
  bh_darray_push(&raylib_source, "src/rtextures.c");
  bh_darray_push(&raylib_source, "src/rtext.c");
  bh_darray_push(&raylib_source, "src/utils.c");
  bh_darray_push(&raylib_source, "src/rmodels.c");
  bh_darray_push(&raylib_source, "src/raudio.c");

  bh_files_t striped_source = { 0 };
  bh_foreach(&raylib_source, source, {
    bh_strings_t strs = { 0 };
    assert(bh_string_to_array(&strs, source, '/'));
    bh_darray_push(&striped_source, strs.buffer[strs.index - 1]);
  });

  bh_async_t async = { 0 };

  for (size_t i = 0; i < bh_darray_len(&striped_source); ++i) {
    char *command =
      bh_fmt(
        "gcc -o bin/%s.o -c %s %s %s",
        striped_source.buffer[i],
        raylib_source.buffer[i],
        bh_files_to_string(&CFLAGS, ' '),
        bh_files_to_string(&INCLUDES, ' ')
      );

    // bh_execute(command);
    bh_push_async(&async, command);
  }

  assert(bh_execute_async(&async));

  bh_arena_free(&arena);

  return 0;
}
