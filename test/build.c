#define BUILD_IMPLEMENTATION
#include "../build.h"

extern bh_arena_t *build_arena;
static bh_arena_t arena = { 0 };

void test_dynamic_array() {
  printf("Testing dynamic arrays...\n");
  
  bh_define_darray(int) int_array_t;
  int_array_t arr = {0};
  
  // Test push
  for (int i = 0; i < 20; i++) {
    bh_darray_push(&arr, i);
  }
  assert(bh_darray_len(&arr) == 20);
  
  // Test get
  for (int i = 0; i < 20; i++) {
    assert(bh_darray_get(&arr, i) == i);
  }
  
  // Test pop
  bh_darray_pop(&arr);
  assert(bh_darray_len(&arr) == 19);
  
  // Test reset
  bh_darray_reset(&arr);
  assert(bh_darray_len(&arr) == 0);
  
  // Test push multiple
  int items[] = {1,2,3,4,5};
  bh_darray_push_mul(&arr, items, 5);
  assert(bh_darray_len(&arr) == 5);
  
  bh_darray_free(&arr);
  printf("Dynamic array tests passed!\n\n");
}

void test_string_operations() {
  printf("Testing string operations...\n");
  
  // Test string join
  char *joined = bh_string_join("hello", "world");
  assert(strcmp(joined, "helloworld") == 0);
  
  // Test string chop
  char *chopped = bh_string_chop("hello world", 0, 5);
  assert(strcmp(chopped, "hello") == 0);
  
  chopped = bh_string_chop("hello world", 6, 11);
  assert(strcmp(chopped, "world") == 0);
  
  // Test string replace
  char *to_replace = bh_string_join("a-b-c", "-d");
  char *replaced = bh_string_replace_char(to_replace, '-', '_');
  assert(strcmp(replaced, "a_b_c_d") == 0);
  
  // Test string to array
  bh_strings_t strings = {0};
  assert(bh_string_to_array(&strings, "one,two,three", ','));
  assert(bh_darray_len(&strings) == 3);
  assert(strcmp(strings.buffer[0], "one") == 0);
  assert(strcmp(strings.buffer[1], "two") == 0);
  assert(strcmp(strings.buffer[2], "three") == 0);
  
  bh_darray_free(&strings);
  printf("String operation tests passed!\n\n");
}

void test_file_operations() {
  printf("Testing file operations...\n");
  
  // Create test directory structure
  assert(bh_execute("mkdir -p test_dir/subdir"));
  assert(bh_execute("touch test_dir/file1.txt"));
  assert(bh_execute("touch test_dir/file2.txt"));
  assert(bh_execute("touch test_dir/subdir/file3.txt"));
  
  // Test path existence checks
  assert(bh_path_exist("test_dir") == is_dir);
  assert(bh_path_exist("test_dir/file1.txt") == is_file);
  assert(bh_path_exist("nonexistent") == is_none);
  
  // Test file listing
  bh_files_t files = {0};
  assert(bh_files_get("test_dir", &files));
  assert(bh_darray_len(&files) >= 2); // At least file1 and file2
  
  bh_darray_reset(&files);
  assert(bh_recursive_files_get("test_dir", &files));
  assert(bh_darray_len(&files) >= 3); // Should include subdir files
  
  // Test file read/write
  const char *test_content = "This is a test";
  assert(bh_file_write("test_dir/test_file.txt", test_content, strlen(test_content) + 1));
  
  char *content = bh_file_read("test_dir/test_file.txt");

  assert(content != NULL);
  assert(strcmp(content, test_content) == 0);
  
  // Test file timestamp
  time_t t = bh_file_get_time("test_dir/test_file.txt");
  assert(t != (time_t)(-1));
  
  // Cleanup
  bh_darray_free(&files);
  assert(bh_execute("rm -rf test_dir"));
  printf("File operation tests passed!\n\n");
}

void test_async_operations() {
  printf("Testing async operations...\n");
  
  bh_async_t async = {0};
  
  // Test async execution
  assert(bh_push_async(&async, "sleep 0.1 && touch async_test1.txt"));
  assert(bh_push_async(&async, "sleep 0.1 && touch async_test2.txt"));
  
  // Verify files don't exist yet
  assert(bh_path_exist("async_test1.txt") == is_none);
  assert(bh_path_exist("async_test2.txt") == is_none);
  
  // Wait for completion
  assert(!bh_await(&async));
  
  // Verify files were created
  assert(bh_path_exist("async_test1.txt") == is_file);
  assert(bh_path_exist("async_test2.txt") == is_file);
  
  // Cleanup
  bh_darray_free(&async);
  assert(bh_execute("rm -f async_test1.txt async_test2.txt"));
  printf("Async operation tests passed!\n\n");
}

void test_error_handling() {
  printf("Testing error handling...\n");
  
  bh_try {
    // Simulate an error condition
    if (bh_path_exist("nonexistent_file") == is_none) {
      bh_throw(bh_FileNotFoundError);
    }
  } bh_catch(bh_FileNotFoundError) {
    printf("Caught FileNotFoundError as expected\n");
  }
  
  // Test should continue execution here
  assert(1 == 1);
  printf("Error handling tests passed!\n\n");
}

void test_build_system() {
  printf("Testing build system functionality...\n");
  
  // Create test source file
  const char *src_content = 
    "#include <stdio.h>\n"
    "int main() { printf(\"Hello, world!\\n\"); return 0; }";
  assert(bh_file_write("test_program.c", src_content, strlen(src_content)));
  
  // Test binary outdated check
  bh_files_t files = {0};
  bh_darray_push(&files, "test_program.c");
  
  // Binary doesn't exist yet, should be considered "old"
  assert(bh_is_binary_old("test_program", &files));
  
  // Build the binary
  const char *build_cmd = "cc -o test_program test_program.c";
  assert(bh_execute(build_cmd));
  
  // Binary should now be up-to-date
  assert(!bh_is_binary_old("test_program", &files));
  
  // Modify source file to make binary outdated
  time_t time;

  sleep(1);
  assert(bh_execute("touch test_program.c"));
  assert(bh_is_binary_old("test_program", &files));
  
  // Test conditional build
  assert(bh_on_binary_old_execute("test_program", &files, build_cmd));
  
  // Cleanup
  bh_darray_free(&files);
  assert(bh_execute("rm -f test_program test_program.c"));
  printf("Build system tests passed!\n\n");
}

int main(int argc, char *argv[])
{
  bh_init(argc, argv);

  bh_init_arena(&arena, 1024 * 1024);
  build_arena = &arena;

  test_dynamic_array();
  test_string_operations();
  test_file_operations();
  test_async_operations();
  test_error_handling();
  test_build_system();

  printf("All tests passed successfully!\n");

  bh_arena_free(&arena);

  return 0;
}
