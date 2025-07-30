/********************************************
                 Example Code
*********************************************

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

  bh_arena_free(&arena);

  return 0;
}
********************************************/


/**********************************************************************************************
* build.h (1.3.1) - A simple yet powerful build system written in C.
*
* MIT License
*
* Copyright (c) 2024 Chry003
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
**********************************************************************************************/

#ifndef __build_h__
#define __build_h__

#include <errno.h>
#include <fileapi.h>
#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <synchapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

#if __UNIX__
#include <sys/wait.h>
#elif __WIN32__
#include <windows.h>
#endif

// typedef dynamic array with generic type
#define bh_define_darray(type)  \
  typedef struct {    \
    size_t size;      \
    size_t count;     \
    type *items;     \
  } 

// push item to dynamic array
#define bh_darray_push(ptr, item) do {                                  \
  if ((ptr)->count >= (ptr)->size) {                                    \
    size_t item_size = sizeof(typeof((*(ptr)->items)));                \
    (ptr)->size += 8;                                                   \
    if ((ptr)->items) {                                                \
      (ptr)->items = realloc((ptr)->items, (ptr)->size * item_size);  \
    } else {                                                            \
      (ptr)->items = malloc((ptr)->size * item_size);                  \
    }                                                                   \
  }                                                                     \
  (ptr)->items[(ptr)->count++] = item;                                 \
} while(0)

#define bh_darray_pop(ptr) ((ptr)->count > 0 ? (ptr)->count-- : 0)

#define bh_darray_get(ptr, index) ((ptr)->items[index])

// push multiple items to dynamic array
#define bh_darray_push_mul(ptr, items, count) do {  \
  for (size_t i = 0; i < count; ++i) {              \
    bh_darray_push(ptr, items[i]);                  \
  }                                                 \
} while(0)

// len of dynamic array
#define bh_darray_len(ptr) (ptr)->count

#define bh_darray_drop(in_dptr, out_dptr, n) do {   \
  for (size_t i = n; i < (in_dptr)->count; ++i) {   \
    bh_darray_push(out_dptr, (in_dptr)->items[i]); \
  }                                                 \
} while(0)

#define bh_darray_take(in_dptr, out_dptr, n) do {                               \
  for (size_t i = 0; i < (n < (in_dptr)->count ? n : (in_dptr)->count); ++i) {  \
    bh_darray_push(out_dptr, (in_dptr)->items[i]);                             \
  }                                                                             \
} while(0)

// reset dynamic array
#define bh_darray_reset(ptr) (ptr)->count = 0

// free allocated memory in dynamic array
#define bh_darray_free(ptr) do {  \
  if ((ptr)->items) {            \
    free((ptr)->items);          \
  }                               \
} while(0)

#if __UNIX__
#define bh_async(async_ptr, expr) ({                \
  bool result = true;                               \
  pid_t pid = fork();                               \
  if (pid < 0) result = false;                      \
  else if (pid == 0) { exit(expr); }                \
  bh_darray_push(async_ptr, ((bh_command_t) {       \
    .pid = pid,                                     \
    .command = NULL                                 \
  }));                                              \
  (result);                                         \
})
#elif __WIN32__
#define bh_async(async_ptr, expr) ({                \
  bool result = true;                               \
  STARTUPINFO si = { sizeof(si) };                  \
  PROCESS_INFORMATION pi;                           \
  char cmdline[] = "cmd /c exit " #expr;            \
                                                    \
  if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) { \
    result = false;                                 \
  } else {                                          \
    CloseHandle(pi.hThread);                        \
    bh_darray_push(async_ptr, ((bh_command_t) {     \
      .pid = pi.hProcess,                           \
      .command = NULL                               \
    }));                                            \
  }                                                 \
  (result);                                         \
})
#endif

#define bh_foreach(ptr, closure, body) do { \
  for (size_t i = 0; i < bh_darray_len(ptr); ++i) { \
    typeof(*((ptr)->items)) closure = (ptr)->items[i]; \
    body; \
  } \
} while(0)

#define bh_filter(in_dptr, out_dptr, fptr) do {                        \
  for (size_t i = 0; i < (in_dptr)->count; ++i) {                   \
    typeof(*(out_dptr)->items) item = fptr((in_dptr)->items[i]);  \
    if (item) bh_darray_push(out_dptr, item);                          \
  }                                                                 \
} while(0)

#define bh_map(in_dptr, out_dptr, fptr) do {               \
  for (size_t i = 0; i < (in_dptr)->count; ++i) {       \
    bh_darray_push(out_dptr, fptr((in_dptr)->items[i]));  \
  }                                                     \
} while(0)

#define bh_throw(k) { bh_current_err_state = k; break; }
#define bh_uthrow(ptr, k) if (!ptr) bh_throw(k)
#define bh_try do { bh_current_err_state = bh_NoError;
#define bh_catch(k) } while(0); if (bh_current_err_state == k)

typedef enum {
  bh_NoError = 0,
  bh_FileNotFoundError,
} bh_error_type_t;

static int bh_current_err_state = bh_NoError;

typedef struct {
#if __UNIX__
  pid_t pid;
#elif __WIN32__
  HANDLE pid;
#endif
  char *command;
} bh_command_t;

bh_define_darray(bh_command_t) bh_async_t;
bh_define_darray(char *) bh_files_t;
typedef bh_files_t bh_strings_t;

typedef enum {
	BLACK 	= 0,
	RED 		= 1,
	GREEN 	= 2,
	YELLOW 	= 3,
	BLUE 		= 4,
	MAGENTA = 5,
	CYAN 		= 6,
	WHITE 	= 7
} bh_term_color_t;

typedef enum {
	TEXT = 0,
	BOLD_TEXT,
	UNDERLINE_TEXT,
	BACKGROUND,
	HIGH_INTEN_BG,
	HIGH_INTEN_TEXT,
	BOLD_HIGH_INTEN_TEXT,
	RESET
} bh_term_kind_t;

typedef struct {
  size_t size;
  size_t offset;
  char *buffer;
} bh_arena_t;

typedef enum {
  is_file = 0,
  is_dir,
  is_none
} bh_path_kind_t;

static bh_arena_t *build_arena;

// string formating
#define bh_fmt(...) ({ bh_fmt_fn(__VA_ARGS__, NULL); })

char *bh_fmt_fn(char *s, ...);
void bh_log(int level, const char *message);

void bh_init_arena(bh_arena_t *arena, size_t size);
void *bh_arena_alloc(bh_arena_t *arena, size_t size);
#define bh_arena_reset(arena) (arena)->offset = 0
#define bh_arena_free(arena) do { \
  if (arena) {                    \
    if ((arena)->buffer)          \
      free((arena)->buffer);      \
  }                               \
} while(0)

bool bh_mkdir(const char *path);
bh_path_kind_t bh_path_exist(const char *path);
bool bh_dir_get(const char *path, bh_files_t *dirs);

char *bh_files_to_string(bh_files_t *files, const unsigned char seperator);
bool bh_files_get(const char *path, bh_files_t *files);
bool bh_recursive_files_get(const char *path, bh_files_t *files);
char *bh_file_read(const char *path);
bool bh_file_write(const char *path, const char *buffer, size_t size);
time_t bh_file_get_time(const char *path);

char *bh_string_join(const char *f, const char *s);
char *bh_string_chop(const char *s, size_t from, size_t to);
char *bh_string_replace_char(char *s, char from, char to);

bool bh_string_to_array(bh_strings_t *strings, const char *string, const unsigned char seperator);

bool bh_execute(const char *command);
bool bh_is_binary_old(const char *bin_path, bh_files_t *files);
bool bh_on_binary_old_execute(const char *bin_path, bh_files_t *files, const char *command);
bool bh_push_async(bh_async_t *async, const char *command);
bool bh_await(bh_async_t *async);

// C specific stuff
bool bh_c_source_get_include_paths(
  bh_files_t *include_paths,
  const char *source,
  const char *args,
  const char *out_filename);

#ifdef BUILD_IMPLEMENTATION

char *bh_fmt_fn(char *s, ...)
{
	// allocate small size buffer
	size_t buffer_size = 64; // bytes
	char *buffer = (char *)bh_arena_alloc(build_arena, buffer_size);

	if (buffer == NULL) {
		perror("failed to allocate memory for formating string: ");
		return NULL;
	}

	va_list ap;
	va_start(ap, s);

	size_t nSize = vsnprintf(buffer, buffer_size, s, ap);
	if (!nSize) {
		va_end(ap);
	}

	// if buffer does not have enough space then extend it.
	if (nSize >= buffer_size) {
		buffer_size = nSize + 1;
		buffer = (char*)bh_arena_alloc(build_arena, buffer_size);

		if (buffer == NULL) {
			perror("failed to extend buffer for allocating formated string: ");
			va_end(ap);

			return NULL;
		}

		va_start(ap, s);
		vsnprintf(buffer, buffer_size, s, ap);
	}

	va_end(ap);

	return buffer;
}

void bh_log(int level, const char *message)
{
  switch (level) {
    case 1: fprintf(stdout, "[INFO]: %s", message); break;
    case 2: fprintf(stdout, "[WARN]: %s", message); break;
    case 3: fprintf(stderr, "[ERROR]: %s", message); break;
  }
}

void bh_init_arena(bh_arena_t *arena, size_t size)
{
  (*arena).buffer = (char *)malloc(size);
  (*arena).size = size;
}

void *bh_arena_alloc(bh_arena_t *arena, size_t size)
{
  assert(arena->offset + size <= arena->size);
  void *block = arena->buffer + arena->offset;
  arena->offset += size;
  return block;
}

bool bh_mkdir(const char *path)
{
	size_t pos = 0;
	for (size_t i = 0; i < strlen(path); ++i) {
		if (path[i] == '/') {
			char *pre = bh_string_chop(path, 0, pos);
			char *sub = bh_string_chop(path, pos, i);

			if (sub) {
				const char *final_dir_path = pre ?
					bh_fmt((char *)"mkdir %s%s", pre, sub) :
					bh_fmt((char *)"mkdir %s", sub);

				if (bh_path_exist(bh_string_chop(final_dir_path, 6, strlen(final_dir_path))) != is_dir) {
          if (!bh_execute(final_dir_path)) return false;
        }
			}

			pos = i + 1;
		}
	}

	char *pre = bh_string_chop(path, 0, pos);
	char *sub = bh_string_chop(path, pos, strlen(path));

	if (sub) {
		const char *final_dir_path = pre ?
			bh_fmt((char *)"mkdir %s%s", pre, sub) :
			bh_fmt((char *)"mkdir %s", sub);

		if (bh_path_exist(bh_string_chop(final_dir_path, 6, strlen(final_dir_path))) != is_dir) {
      if (!bh_execute(final_dir_path))
        return false;
    }
	}

  return true;
}

bh_path_kind_t bh_path_exist(const char *path)
{
  bh_path_kind_t kind = is_none;

#if __UNIX__
  struct stat bf;

  if (stat(path, &bf)) {
    return kind;
  }

  if (S_ISREG(bf.st_mode)) kind = is_file;
  else if (S_ISDIR(bf.st_mode)) kind = is_dir;

#elif __WIN32__
  DWORD attrib = GetFileAttributes(path);
  if (attrib == INVALID_FILE_ATTRIBUTES)
    return kind;

  if (attrib & FILE_ATTRIBUTE_DIRECTORY)
    kind = is_dir;
  else
    kind = is_file;

#endif

  return kind;
}

bool bh_dir_get(const char *path, bh_files_t *dirs)
{
  bh_path_kind_t kind = bh_path_exist(path);
  if (kind == is_none || kind == is_file) {
    bh_log(3, "expected directory path.\n");
    return false;
  }

  bool has_slash = false;
  char slash = bh_string_chop(path, strlen(path) - 1, strlen(path))[0];

  if (slash == '/' || slash == '\\')
    has_slash = true;
  else
#if __UNIX__
    slash = '/';
#elif __WIN32__
    slash = '\\';
#endif

  DIR *dir = opendir(path);
  if (dir == NULL) {
    bh_log(3, bh_fmt("failed to open directory, `%s`.\n", path));
    return false;
  }

  struct dirent *data;
  while ((data = readdir(dir)) != NULL)
  {
    char *next_path = bh_fmt("%s%c%s", path, slash, data->d_name);

#if __UNIX__
    if (data->d_type == DT_DIR)
    {
      bh_darray_push(dirs, next_path);
    }
#elif __WIN32__
    if ((strcmp(data->d_name, "..") && strcmp(data->d_name, ".")) &&
      bh_path_exist(next_path) == is_dir)
    {
      bh_darray_push(dirs, next_path);
    }
#endif
  }

  return true;
}

char *bh_files_to_string(bh_files_t *files, const unsigned char seperator)
{
	size_t total_size_of_string = 1;

	for (size_t i = 0; i < bh_darray_len(files); ++i)
  {
		total_size_of_string += strlen(files->items[i]);
    if (i != bh_darray_len(files) - 1 && seperator) {
      total_size_of_string++;
    }
  }

	char *string = (char *)malloc(total_size_of_string);
  if (!string) return NULL;

	size_t pos = 0;
	for (size_t i = 0; i < bh_darray_len(files); ++i)
	{
    size_t len = strlen(files->items[i]);
    strncpy(string + pos, files->items[i], len);
    pos += len;

    if (i != bh_darray_len(files) - 1 && seperator) {
      string[pos] = seperator;
      pos++;
    }
	}

	string[pos] = '\0';

	return string;
}

bool bh_files_get(const char *path, bh_files_t *files)
{
  bh_path_kind_t kind = bh_path_exist(path);
  if (kind == is_none || kind == is_file) {
    bh_log(3, "expected directory path.\n");
    return false;
  }

	bool has_slash = false;
	char slash = bh_string_chop(path, strlen(path) - 1, strlen(path))[0];

	if (slash == '/' || slash == '\\')
		has_slash = true;

	DIR *dir = opendir(path);
	if (dir == NULL) {
    bh_log(3, bh_fmt("failed to open directory, `%s`.\n", path));
    return false;
  }

	struct dirent *data;
	while ((data = readdir(dir)) != NULL)
	{

#if __UNIX__
		if (data->d_type != DT_DIR)
		{
			char *fileName = data->d_name;
			char *item = NULL;
			if (has_slash) {
        item = bh_fmt((char *)"%s%s", path, fileName);
      }
			else {
        item = bh_fmt((char *)"%s/%s", path, fileName);
      }

      bh_darray_push(files, item);
		}
#elif __WIN32__
		if (data->d_ino != ENOTDIR && (strcmp(data->d_name, ".") && strcmp(data->d_name, "..")))
		{
			char *fileName = data->d_name;
			char *item = NULL;
			if (has_slash) {
        item = bh_fmt((char *)"%s%s", path, fileName);
      }
			else {
        item = bh_fmt((char *)"%s/%s", path, fileName);
      }

      bh_darray_push(files, item);
		}
#endif
	}

  return true;
}

bool bh_recursive_files_get(const char *path, bh_files_t *files)
{
  bh_path_kind_t kind = bh_path_exist(path);
  if (kind == is_none || kind == is_file) {
    bh_log(3, "expected directory path.\n");
    return false;
  }

	bool has_slash = false;
	char slash = bh_string_chop(path, strlen(path) - 1, strlen(path))[0];

	if (slash == '/' || slash == '\\')
		has_slash = true;

	DIR *dir = opendir(path);
	if (dir == NULL) {
    bh_log(3, bh_fmt("failed to open directory, `%s`.\n", path));
    return false;
  }

	struct dirent *data;
	while ((data = readdir(dir)) != NULL)
	{

#if __UNIX__
    if (data->d_type == DT_DIR) {
      if (strncmp(data->d_name, ".", 1) && strncmp(data->d_name, "..", 2)) {
        const char *npath = (has_slash) ? bh_fmt("%s%s", path, data->d_name) :
          bh_fmt("%s/%s", path, data->d_name);
        if (!bh_recursive_files_get(npath, files))
          return false;
      }
    }
#elif __WIN32__
    if (data->d_ino == EISDIR) {
      if (strncmp(data->d_name, ".", 1) && strncmp(data->d_name, "..", 2)) {
        const char *npath = (has_slash) ? bh_fmt("%s%s", path, data->d_name) :
          bh_fmt("%s/%s", path, data->d_name);
        if (!bh_recursive_files_get(npath, files))
          return false;
      }
    }
#endif
    else {
      char *fileName = data->d_name;
      char *item = NULL;
      if (has_slash) {
        item = bh_fmt((char *)"%s%s", path, fileName);
      }
      else {
        item = bh_fmt((char *)"%s/%s", path, fileName);
      }

      bh_darray_push(files, item);
    }
	}

  return true;

}

char *bh_file_read(const char *path)
{
	FILE *fp;
	long len;

	fp = fopen(path, "rb");
	if (fp == NULL) {
    bh_log(3, bh_fmt("failed to open files, `%s`.\n", path));
		return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char *buffer = (char*)bh_arena_alloc(build_arena, len + 1);
	if (buffer == NULL) {
    bh_log(3, bh_fmt("failed to allocate memory.\n"));
    return NULL;
  }

	fread(buffer, 1, len, fp);
	buffer[len] = 0;
	fclose(fp);

	return buffer;
}

bool bh_file_write(const char *path, const char *buffer, size_t size)
{
	FILE *fp;

  bh_try {
    fp = fopen(path, "wb");
    bh_uthrow(fp, bh_FileNotFoundError);
  } bh_catch(bh_FileNotFoundError) {
    bh_log(3, bh_fmt("failed to open files, `%s`.\n", path));
		return false;
  }

	size_t total_size_written = fwrite(buffer, sizeof(char), size, fp);
  if (total_size_written != size) {
    bh_log(3, bh_fmt("Failed to write complete file `%s`.\n", path));
    fclose(fp);
    return false;
  }

  if (fflush(fp) != 0) {
    bh_log(3, bh_fmt("failed to flush data to file `%s`.\n", path));
    fclose(fp);
    return false;
  }

  if (fclose(fp) != 0) {
    bh_log(3, bh_fmt("failed to properly close file `%s`.\n", path));
    return false;
  }

	return true;
}

time_t bh_file_get_time(const char *path)
{
	struct stat file_stat;
	if (stat(path, &file_stat) == -1) {
		perror(bh_fmt("Failed to get file status: %s, ", path));
		return (time_t)(-1);  // Return -1 on error
	}

	return file_stat.st_mtime;
}

char *bh_string_join(const char *f, const char *s)
{
  size_t size = strlen(f) + strlen(s) + 1;
  char *new_string = (char *)bh_arena_alloc(build_arena, size);

  new_string = strcpy(new_string, f);
  new_string = strcat(new_string, s);
  new_string[size - 1] = '\0';

  return new_string;
}

char *bh_string_chop(const char *s, size_t from, size_t to)
{
	if (from >= to) return NULL;

	char *sub = (char *)bh_arena_alloc(build_arena, (to - from) + 1);
	for (size_t i = 0; i < to - from; ++i)
		sub[i] = s[from + i];

	sub[(to - from)] = '\0';
	
	return sub;
}

char *bh_string_replace_char(char *s, char from, char to)
{
  if (!s) return NULL;

  size_t s_size = strlen(s);

  size_t index = 0;
  char *copy = malloc(s_size + 1);
  memset(copy, 0, s_size + 1);

  for (size_t i = 0; i < s_size; ++i) {
    if (s[i] == from) {
      if (to) copy[index++] = to;
    }
    else copy[index++] = s[i];
  }

  copy[index] = 0;

  return copy;
}

bool bh_string_to_array(bh_strings_t *strings, const char *string, const unsigned char seperator)
{
  if (!string) return false;

  size_t index = 0;
  for (size_t i = 0; i < strlen(string); ++i) {
    if (string[i] == seperator) {
      char *sub = bh_string_chop(string, index, i);
      if (sub) {
        sub[i - index] = 0;
        bh_darray_push(strings, sub);
      }

      index = i + 1;
    }
  }

  char *sub = bh_string_chop(string, index, strlen(string));
  bh_darray_push(strings, sub);

  return true;
}

bool bh_execute(const char *command)
{
	if (command == NULL) return false;
#ifdef BUILD_EXECUTE_LOG
	bh_log(1, bh_fmt("$ %s\n", command));
#endif
	return !system(command);
}

bool bh_is_binary_old(const char *bin_path, bh_files_t *files)
{
	time_t binary_timestamp = bh_file_get_time(bin_path);
	if (binary_timestamp == (time_t)(-1))
		return true;

	for (size_t i = 0; i < bh_darray_len(files); ++i) {
		time_t source_timestamp = bh_file_get_time(files->items[i]);
		if (source_timestamp == (time_t)(-1)) {
      bh_log(3,
        bh_fmt("Failed to get modification time for source file: %s\n",
        files->items[i]
      ));

			continue;
		}

		if (source_timestamp > binary_timestamp)
			return true;
	}

	return false;
}

bool bh_on_binary_old_execute(const char *bin_path, bh_files_t *files, const char *command)
{
  if (bh_is_binary_old(bin_path, files))
    return bh_execute(command);

  return false;
}

#if __UNIX__
bool bh_push_async(bh_async_t *async, const char *command)
{
  pid_t pid = fork();

  if (pid < 0) return false;
  else if (pid == 0) {
    execlp("sh", "sh", "-c", command, NULL);
    bh_log(3, "Async failed");

    return false;
  }

  bh_darray_push(async, ((bh_command_t){
    .pid = pid,
    .command = (char*)command
  }));

  return true;
}
#elif __WIN32__
bool bh_push_async(bh_async_t *async, const char *command)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    WINBOOL success;
    char *cmdline;

    // Create a modifiable command line string
    // Note: CreateProcess may modify the command line string
    cmdline = _strdup(command);
    if (!cmdline) {
        return false;
    }

    success = CreateProcess(
        NULL,           // No application name (use command line)
        cmdline,        // Command line (needs to be writable)
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW, // Don't create a console window
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure
    );

    free(cmdline);  // Free the duplicated command line

    if (!success) {
        bh_log(3, bh_fmt("Async failed (CreateProcess error: %lu)", GetLastError()));
        return false;
    }

    // We don't need the thread handle
    CloseHandle(pi.hThread);

    // Push the process information to the async array
    // Note: You'll need to change bh_command_t to store HANDLE instead of pid_t
    bh_darray_push(async, ((bh_command_t){
        .pid = pi.hProcess,  // Store the process handle
        .command = _strdup(command)  // Duplicate the command string
    }));

    return true;
}
#endif

#if __UNIX__
bool bh_await(bh_async_t *async)
{
  bool ret = false;
  bh_foreach(async, command, {
    if (command.command)
      bh_log(1, bh_fmt("%s\n", command.command));

    int status;
    waitpid(command.pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
      ret = true;
  });

  return ret;
}
#elif __WIN32__
bool bh_await(bh_async_t *async)
{
    bool ret = false;
    bh_foreach(async, command, {
        if (command.command)
            bh_log(1, bh_fmt("%s\n", command.command));

        // Wait for the process to complete
        WaitForSingleObject(command.pid, INFINITE);
        
        // Get the exit code
        DWORD exitCode;
        GetExitCodeProcess(command.pid, &exitCode);
        
        // Clean up the process handle
        CloseHandle(command.pid);
        
        // Check if the process failed
        if (exitCode != EXIT_SUCCESS) {
            ret = true;
        }
        
        // Free the command string if it was allocated
        if (command.command) {
            free(command.command);
        }
    });
    
    // Clear the async array after waiting
    bh_darray_reset(async);
    
    return ret;
}
#endif

bool bh_c_source_get_include_paths(
  bh_files_t *include_paths,
  const char *source,
  const char *args,
  const char *out_filename)
{
  if (!include_paths || !source || !args || !out_filename) return false;

  bh_mkdir(".build_cache");

  char *out = bh_fmt(".build_cache/_csource_includes_%s_.d", out_filename);

  time_t source_time = bh_file_get_time(source);
  time_t out_time = bh_file_get_time(out);

  if (bh_path_exist(out) == is_none || (out_time < source_time)) {
    assert(
      bh_execute(
        bh_fmt(
          "gcc -MM -MF %s %s %s",
          out, source, args
        )
      )
    );
  }

  char *file = bh_file_read(
    bh_fmt("%s", out));

  bh_strings_t items = { 0 };

  bool has_started = false;
  size_t start = 0;
  size_t size = strlen(file);

  for (size_t i = 0; i < size; ++i) {
    char ch = file[i];
    if (ch == ' ') {
      if (!has_started) {
        start = i;
        has_started = true;
      } else {
        char *item = bh_string_chop(file, start, i);
        if (item) bh_darray_push(&items, item);
        start = i;
      }
    }
  }

  char *item = bh_string_chop(file, start, size);
  if (item) bh_darray_push(&items, item);

  char *temp = bh_files_to_string(&items, 0);
  temp = bh_string_replace_char(temp, '\r', 0);
  temp = bh_string_replace_char(temp, '\n', 0);
  temp = bh_string_replace_char(temp, '\\', 0);

  bh_strings_t full_list = { 0 };
  bh_string_to_array(&full_list, temp, ' ');

  bh_darray_drop(&full_list, include_paths, 1);

  return true;
}

void bh_init(int argc, char *argv[])
{
  bh_arena_t arena = { 0 };
  bh_init_arena(&arena, 1024);
  build_arena = &arena;

  char *bin = argv[0];

#if __UNIX__
  char *src = bh_string_join(bin, ".c");
#elif __WIN32__
  char *src = bh_string_join(bh_string_chop(bin, 0, strlen(bin) - 4), ".c");
#endif

  bh_files_t files = { 0 };
  bh_darray_push(&files, src);
  bh_c_source_get_include_paths(&files, src, "", "build_include");

  const char *command = bh_fmt("gcc -o %s %s -march=native -O3", bin, src);

  if (bh_is_binary_old(bin, &files)) {

#if __WIN32__
    if (!MoveFileEx(bin, bh_fmt("%s.old", bin), MOVEFILE_REPLACE_EXISTING)) {
      bh_log(3, bh_fmt("Could not rename `%s` to `%s.old`.\n", bin, bin));
    }

    if (!bh_execute(command)) {
      if (!MoveFileEx(bh_fmt("%s.old", bin), bin, MOVEFILE_REPLACE_EXISTING)) {
        bh_log(3, bh_fmt("Could not rename `%s` to `%s.old`.\n", bin, bin));
      }
    }
#else
    bh_execute(command);
#endif

    bh_darray_free(&files);
    bh_arena_free(&arena);
    build_arena = NULL;
    exit(0);
  }

  bh_darray_free(&files);
  bh_arena_free(&arena);
  build_arena = NULL;
}

#endif // BUILD_IMPLEMENTATION
#endif //__build_h__
