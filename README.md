# build.h - A Simple Yet Powerful Build System in C

![License](https://img.shields.io/badge/license-MIT-blue.svg)

`build.h` is a single-header build system written in C that provides essential tools for file operations, process management, and build automation.

## Features

- **File Operations**: Read/write files, check file existence, get modification times
- **Directory Handling**: List files recursively/non-recursively, create directories
- **String Manipulation**: Join, split, replace characters in strings
- **Process Management**: Execute commands, async command execution with await
- **Build Automation**: Check if binaries need rebuilding based on source timestamps
- **Memory Management**: Arena allocator for efficient memory handling
- **Error Handling**: Simple error handling mechanism
- **Logging**: Basic logging functionality

## Quick Start

1. Include `build.h` in your project
2. Define `BUILD_IMPLEMENTATION` in one C file before including
3. Use the provided functions

```c
#define BUILD_IMPLEMENTATION
#include "build.h"

int main() {
    bh_init_auto();  // Optional auto-build setup
    
    // Your build logic here
    bh_files_t sources = {0};
    bh_recursive_files_get("src", &sources);
    
    const char* cmd = bh_fmt("cc -o myapp %s", bh_files_to_string(&sources, ' '));
    bh_execute(cmd);
    
    bh_darray_free(&sources);
    return 0;
}
```

## API Reference
### Core Functions

- `bh_init_arena()` - Initialize memory arena
- `bh_arena_alloc()` - Allocate memory from arena
- `bh_fmt()` - String formatting (similar to printf)
- `bh_log()` - Log messages with different levels

### File Operations

- `bh_file_read()` - Read entire file
- `bh_file_write()` - Write to file
- `bh_file_get_time()` - Get file modification time
- `bh_path_exist()` - Check if path exists and its type
- `bh_mkdir()` - Create directory (including parents)

### Directory Handling
- `bh_files_get()` - List files in directory
- `bh_recursive_files_get()` - Recursively list files
- `bh_files_to_string()` - Join filenames with separator

### String Utilities

- `bh_string_join()` - Concatenate two strings
- `bh_string_chop()` - Extract substring
- `bh_string_replace_char()` - Replace characters in string
- `bh_string_to_array()` - Split string by separator

### Process Management

- `bh_execute()` - Execute shell command
- `bh_push_async()` - Run command asynchronously
- `bh_await()` - Wait for async commands to complete
- `bh_is_binary_old()` - Check if binary is older than sources
- `bh_on_binary_old_execute()` - Conditional command execution

## Dynamic Arrays

### Macros for type-safe dynamic arrays:

- `bh_define_darray()` - Define new dynamic array type
- `bh_darray_push()` - Add item to array
- `bh_darray_pop()` - Remove last item
- `bh_darray_get()` - Access item by index
- `bh_darray_len()` - Get array length
- `bh_darray_free()` - Free array memory

## Error Handling

```c
bh_try {
    FILE* fp = fopen("missing.txt", "r");
    bh_uthrow(fp, bh_FileNotFoundError);
} bh_catch(bh_FileNotFoundError) {
    bh_log(3, "File not found!");
}
```

## Auto-Build Feature

- `build.h` can automatically rebuild itself when modified:

```c
int main(int argc, char* argv[]) {
    bh_init(argc, argv);  // Checks if rebuild is needed
    
    // Rest of your program
    return 0;
}
```

## Examples
- Basic Build Script

```c
#define BUILD_IMPLEMENTATION
#include "build.h"

int main() {
    bh_files_t sources = {0};
    bh_recursive_files_get("src", &sources);
    
    const char* cmd = bh_fmt("cc -o myapp %s -Wall -Wextra", 
                           bh_files_to_string(&sources, ' '));
    bh_execute(cmd);
    
    bh_darray_free(&sources);
    return 0;
}
```

## Parallel Build

```c
bh_async_t async = {0};
bh_push_async(&async, "cc -c file1.c -o file1.o");
bh_push_async(&async, "cc -c file2.c -o file2.o");
bh_await(&async);
```

License

MIT License - see LICENSE file for details.
Contributing

Contributions are welcome! Please open issues or pull requests for any bugs or feature requests.
