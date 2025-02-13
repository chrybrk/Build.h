/**********************************************************************************************
* build.h (1.0.0) - A simple yet powerful build system written in C.
*
* FEATURES:
* 	- No external dependencies.
* 	- Automatic memory management using arena allocator.
* 	- Automatic updates binary of build system: once compiled upon running it will check for any updates in the build-system.
* 	- Arena allocator and Dynamic array.
* 	- String function: join, seperate, sub-string.
* 	- Get list of files, Create directories, check if files has been modified.
* 	- Execute commands with strings, fromated strings.
*
* NOTES:
*
* 	#define IMPLEMENT_BUILD_H
* 	#include "build.h"
* 	
*		const char *build_source;
*		const char *build_bin;
*
*		These two must be set in the source file.
*
*		void build_start(int argc, char **argv)
*		{
*		}
*
*		This is the function that will start executing.
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

#define IMPLEMENT_BUILD_H
#ifdef IMPLEMENT_BUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/wait.h>

// called at the end of scope
#define defer(func) __attribute__((cleanup(func)))

// string formating
#define formate_string(...) ({ __formate_string_function__(__VA_ARGS__, NULL); })

// log-system
#define INFO(...) printf("%s[BUILD :: INFO]:%s %s\n", get_term_color(TEXT, GREEN), get_term_color(RESET, 0), formate_string(__VA_ARGS__))
#define WARN(...) printf("%s[BUILD :: WARN]:%s %s\n", get_term_color(TEXT, YELLOW), get_term_color(RESET, 0), formate_string(__VA_ARGS__))
#define ERROR(...) printf("%s[BUILD :: ERROR]:%s %s\n", get_term_color(TEXT, RED), get_term_color(RESET, 0), formate_string(__VA_ARGS__)), exit_build_system(1);

typedef enum {
	BLACK 	= 0,
	RED 		= 1,
	GREEN 	= 2,
	YELLOW 	= 3,
	BLUE 		= 4,
	MAGENTA = 5,
	CYAN 		= 6,
	WHITE 	= 7
} TERM_COLOR;

typedef enum {
	TEXT = 0,
	BOLD_TEXT,
	UNDERLINE_TEXT,
	BACKGROUND,
	HIGH_INTEN_BG,
	HIGH_INTEN_TEXT,
	BOLD_HIGH_INTEN_TEXT,
	RESET
} TERM_KIND;

// arena allocator
typedef struct ARENA_STRUCT
{
	size_t buffer_size;		// size of buffer
	void *buffer;					// buffer
	size_t ptr;						// current pointer in buffer
} arena_T;

// array
typedef struct ARRAY_STRUCT
{
	void **buffer;				// buffer to store elements
	size_t index;					// index (pointer) where we need to store (actual length of array)
	size_t len;						// length of the buffer (not true length)
	size_t item_size;			// size of the item that needs to be put
} array_T;

/*
 * This hash library is inspired (copied) from this reference.
 * https://benhoyt.com/writings/hash-table-in-c/
 *
 * Legal Notice:
 * You may use however you want to, including the right godfather.
 *
 * It also uses `djb2` hashing function. 
 * reference: http://www.cse.yorku.ca/~oz/hash.html
*/

/*
 * hash struct consist of capacity, length and buffer.
 * it is a pretty simple and straight forward hash library.
*/
#define MAX_HASH_TABLE_CAPACITY 1024

typedef struct {
	const char *key;
	void *value;
} hash_entry;

typedef struct HASH_STRUCT {
	hash_entry *buffer;
	ssize_t capacity;
	ssize_t length;
	char **keys;
} hash_T;

/*
 * Hashing function
 * 
 * It is a djb2 hashing function.
 * reference: http://www.cse.yorku.ca/~oz/hash.html
 *
 * Legal Notice:
 * I am not the godfather of this hashing function but if it works on your project, use it.
*/
unsigned long long
hash_function(const char *string)
{
	// magic number is supposed to be `33`.
	unsigned long hash = 5381;
	int c;

	while ((c = *string++) != 0)
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

/*
 * init hash table with default capacity,
 * it still needs improvement for allocating more than default capacity.
 *
 * TODO: More capacity.
*/
hash_T *init_hash();

/*
 * it will free the buffer first and then free the hash itself.
 * It may need to free all the keys, as in `benhoyt` implementation.
 *
 * But, for my use case i think it is fine to leave it as it is.
*/
void hash_free(hash_T *hash);

/*
 * It will return list of keys,
 * that are present int the hashmap.
*/
const char **hash_bucket(hash_T *hash);

/*
 * It will set the value if it founds the key in entry,
 * else it will create new entry.
*/
void hash_set(hash_T *hash, const char *key, void *value);

/*
 * it will loop through from hash_index till capacity.
 * if it found non-null value, it will return.
 * else null will be returned
*/
void* hash_get(hash_T *hash, const char *key);

/**********************************************************************************************
*																   Things that needs to be global
**********************************************************************************************/

// function that needs to implemented by the source
extern void build_start(int argc, char **argv);

// build files
extern const char *build_source;
extern const char *build_bin;

// warn for non-freed arrays 
static int array_allocated_counter = 0;

// color for logging
const char *get_term_color(TERM_KIND kind, TERM_COLOR color);

// macro-related function for formating string
char *__formate_string_function__(char *s, ...);

// arena that will be used for allocating most of the things.
static arena_T *build_main_arena;

// create arena allocator
arena_T *init_arena(size_t n);

// allocate buffer on arena
void *arena_allocate(arena_T *arena, size_t n);

// free the allocator
void arena_free(arena_T **arena);

// create new array
array_T *init_array(size_t item_size);

// push new element in array
void array_push(array_T *array, void *item);

// pop last element from the array
void *array_pop(array_T *array);

// get the element from the array
void *array_get(array_T *array, size_t index);

// take n element from array
array_T *array_take(array_T *array, unsigned int n);

// drop n element from array
array_T *array_drop(array_T *array, unsigned int n);

// merge two array
array_T *array_merge(array_T *a1, array_T *a2);

// free array
void array_free(array_T *array);

// string join
char *strjoin(const char *s0, const char *s1);

// string sub-string
char *strsub(const char *s, size_t fp, size_t tp);

// string replace
char *strreplace(char *s, unsigned char from, unsigned char to);

// convert string to array
array_T *strtoarray(const char *s, unsigned char sep);

// convert (const char **) to (const char *)
const char *strconvCCAtoCC(const char **CCA, size_t len, unsigned char sep);

// convert array to string
const char *strconvCAtoCC(array_T *array, unsigned sep);

// (if) string in array
bool strinarray(const char *s, array_T *array);

// exectue command in the shell
bool command_execute(const char *command);

// get list of files
array_T *file_get(const char *path);

// get list of files (ends with)
array_T *file_get_end(const char *path, const char *end);

// check if file exist
bool file_exist(const char *path);

// check if directory exist
bool directory_exist(const char *path);

// create new directory (it will create all directory in the path)
void create_directory(const char *path);

// get last modified time of file
time_t get_time_from_file(const char *path);

// checks if binary source has changed
bool binary_test(const char *binary_path, array_T *source_list);

/**********************************************************************************************
*																   The Actual Implementation
**********************************************************************************************/

const char *get_term_color(TERM_KIND kind, TERM_COLOR color)
{
	switch (kind)
	{
		case TEXT: return formate_string("\e[0;3%dm", color);
		case BOLD_TEXT: return formate_string("\e[1;3%dm", color);
		case UNDERLINE_TEXT: return formate_string("\e[4;3%dm", color);
		case BACKGROUND: return formate_string("\e[4%dm", color);
		case HIGH_INTEN_BG: return formate_string("\e[0;10%dm", color);
		case HIGH_INTEN_TEXT: return formate_string("\e[0;9%dm", color);
		case BOLD_HIGH_INTEN_TEXT: return formate_string("\e[1;9%dm", color);
		case RESET: return formate_string("\e[0m");
	}
}

char *__formate_string_function__(char *s, ...)
{
	// allocate small size buffer
	size_t buffer_size = 64; // bytes
	char *buffer = malloc(buffer_size);

	if (buffer == NULL)
	{
		perror("failed to allocate memory for formating string: ");
		return NULL;
	}

	va_list ap;
	va_start(ap, s);

	size_t nSize = vsnprintf(buffer, buffer_size, s, ap);
	if (nSize < 0)
	{
		free(buffer);
		va_end(ap);
	}

	// if buffer does not have enough space then extend it.
	if (nSize >= buffer_size)
	{
		buffer_size = nSize + 1;
		buffer = (char*)realloc(buffer, buffer_size);

		if (buffer == NULL)
		{
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

arena_T *init_arena(size_t n)
{
	if (n <= 0) return NULL;

	arena_T *new = malloc(sizeof(struct ARENA_STRUCT));
	if (new == NULL)
	{
		perror("failed to allocate memory for arena: ");
		return NULL;
	}

	new->buffer_size = n;
	new->buffer = malloc(new->buffer_size);
	new->ptr = 0;

	if (new->buffer == NULL)
	{
		perror("failed to allocate memory for arena buffer: ");

		fprintf(stderr, "trying to allocate less memory for arena buffer.\n");

		while (new->buffer_size > (1 * 1024))
		{
			new->buffer_size /= 2;
			fprintf(stderr, "allocating %zu for arena buffer.\n", new->buffer_size);

			new->buffer = malloc(new->buffer_size);
			if (new->buffer) return new;
		}

		fprintf(stderr, "failed to allocate less-memory for arena.\n");
		return NULL;
	}

	return new;
}

void *arena_allocate(arena_T *arena, size_t n)
{
	if (arena->ptr + n < arena->buffer_size)
	{
		void *addr = arena->buffer + arena->ptr;
		arena->ptr += n;

		return addr;
	}

	fprintf(stderr, "max memory reached for arena.\n");
	return NULL;
}

void arena_free(arena_T **arena)
{
	if (*arena)
		free(((arena_T*)*arena)->buffer);
}

array_T *init_array(size_t item_size)
{
	array_T *array = malloc(sizeof(struct ARRAY_STRUCT));
	if (array == NULL)
	{
		perror("failed to allocate memory for array: ");
		return NULL;
	}

	// initially allocate enough for 10 elements.
	array->len = 10;
	array->item_size = item_size;
	array->buffer = malloc(array->item_size * array->len);
	if (array->buffer == NULL)
	{
		perror("failed to allocate memory for array buffer: ");
		return NULL;
	}

	array->index = 0;
	array_allocated_counter++;

	return array;
}

void array_push(array_T *array, void *item)
{
	if (item == NULL) return;

	if (array->buffer == NULL)
	{
		fprintf(stderr, "cannot push element buffer is not allocated.\n");
		return;
	}

	if (array->index >= array->len)
		array->buffer = realloc(array->buffer, (array->len + 10) * array->item_size);

	array->buffer[array->index++] = item;
}

void *array_pop(array_T *array)
{
	if (array->index <= 0) return NULL;
	return array->buffer[--array->index];
}

void *array_get(array_T *array, size_t index)
{
	if (index >= array->index) return NULL;
	return array->buffer[index];
}

array_T *array_take(array_T *array, unsigned int n)
{
	array_T *n_array = init_array(array->item_size);

	for (size_t i = 0; i < n; ++i)
	{
		void *item = array_get(array, i);
		if (item)
			array_push(n_array, item);
	}

	return n_array;
}

array_T *array_drop(array_T *array, unsigned int n)
{
	array_T *n_array = init_array(array->item_size);

	for (size_t i = n; i < array->index; ++i)
	{
		void *item = array_get(array, i);
		if (item)
			array_push(n_array, item);
	}

	return n_array;
}

array_T *array_merge(array_T *a1, array_T *a2)
{
	if (a1->item_size != a2->item_size)
	{
		printf("both array must have similiar array item size.\n");
		return NULL;
	}

	array_T *na = init_array(a1->item_size);
	for (size_t i = 0; i < a1->index; ++i)
		array_push(na, array_get(a1, i));

	for (size_t i = 0; i < a2->index; ++i)
		array_push(na, array_get(a2, i));

	return na;
}

// TODO(NOTE): it is not well tested, it might have memory leaks.
void array_free(array_T *array)
{
	if (array->buffer)
	{
		free(array->buffer);
		array_allocated_counter--;
	}
}

char *strjoin(const char *s0, const char *s1)
{
	char *s = arena_allocate(
			build_main_arena,
			strlen(s0) + strlen(s1) + 1
	);

	s = strcpy(s, s0);
	s = strcat(s, s1);

	return s;
}

char *strsub(const char *s, size_t fp, size_t tp)
{
	if (fp >= tp) return NULL;

	char *sub = arena_allocate(build_main_arena, (tp - fp) + 1);
	for (size_t i = 0; i < tp - fp; ++i)
		sub[i] = s[fp + i];

	sub[(tp - fp)] = '\0';
	
	return sub;
}

char *strreplace(char *s, unsigned char from, unsigned char to)
{
	for (size_t i = 0; i < strlen(s); ++i)
		if (s[i] == from)
			s[i] = to;

	return s;
}

array_T *strtoarray(const char *s, unsigned char sep)
{
	array_T *array = init_array(sizeof(const char *));

	size_t pos = 0;
	for (size_t i = 0; i < strlen(s); ++i)
	{
		if (s[i] == sep)
		{
			char *string = strsub(s, pos, i);
			array_push(array, string);
			pos = i + 1;
		}
	}

	char *string = strsub(s, pos, strlen(s));
	array_push(array, string);

	return array;
}

const char *strconvCCAtoCC(const char **CCA, size_t len, unsigned char sep)
{
	size_t total_size_of_string = 0;

	for (size_t i = 0; i < len; ++i)
		total_size_of_string += strlen(CCA[i]) + 1;

	char *string = arena_allocate(build_main_arena, total_size_of_string + 1);
	size_t pos = 0;
	for (size_t i = 0; i < len; ++i)
	{
		string = strcat(string, CCA[i]);
		pos += strlen(CCA[i]) + 1;
		string[pos - 1] = sep;
	}

	string[total_size_of_string] = '\0';

	return string;
}

const char *strconvCAtoCC(array_T *array, unsigned sep)
{
	size_t total_size_of_string = 0;

	for (size_t i = 0; i < array->index; ++i)
	{
		const char *s = array->buffer[i];
		total_size_of_string += strlen(s) + 1;
	}

	char *string = arena_allocate(build_main_arena, total_size_of_string + 1);
	size_t pos = 0;
	for (size_t i = 0; i < array->index; ++i)
	{
		const char *s = array->buffer[i];
		string = strcat(string, s);
		pos += strlen(s) + 1;
		string[pos - 1] = sep;
	}

	string[total_size_of_string] = '\0';

	return string;
}

bool strinarray(const char *s, array_T *array)
{
	for (size_t i = 0; i < array->index; ++i)
	{
		if (!strcmp(s, array_get(array, i)))
			return true;
	}

	return false;
}

bool command_execute(const char *command)
{
	if (command == NULL) return false;

	return system(command);
}

array_T *file_get(const char *path)
{
	bool hash_slash = false;
	char slash = strsub(path, strlen(path) - 1, strlen(path))[0];

	if (slash == '/' || slash == '\\')
		hash_slash = true;

	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		perror(formate_string("Directory `%s` does not exist.", path));
		return NULL;
	}

	array_T *buffer = init_array(sizeof(const char *));

	struct dirent *data;
	while ((data = readdir(dir)) != NULL)
	{
#		if __unix__
		if (data->d_type != DT_DIR)
#		elif __WIN32__
		if (data->d_ino != ENOTDIR && (strcmp(data->d_name, ".") && strcmp(data->d_name, "..")))
#		endif
		{
			char *fileName = data->d_name;
			if (hash_slash)
				array_push(buffer, (void*)formate_string("%s%s", path, fileName));
			else
#				if __unix__
					array_push(buffer, (void*)formate_string("%s/%s", path, fileName));
#				elif __WIN32__
					array_push(buffer, (void*)formate_string("%s\\%s", path, fileName));
#				endif
		}
	}

	return buffer;
}

array_T *file_get_end(const char *path, const char *end)
{
	array_T *files = file_get(path);
	array_T *files_with_excep = init_array(sizeof(const char *));

	for (size_t i = 0; i < files->index; ++i)
	{
		char *file = files->buffer[i];
		char *substr = strsub(file, strlen(file) - strlen(end), strlen(file));

		if (!strcmp(substr, end))
			array_push(files_with_excep, file);
	}

	array_free(files);
	return files_with_excep;
}

bool file_exist(const char *path)
{
	FILE *file = fopen(path, "r");
	if (file == NULL) return false;

	free(file);
	return true;
}

bool directory_exist(const char *path)
{
	DIR *dir = opendir(path);
	if (dir == NULL) return false;

	free(dir);
	return true;
}

void create_directory(const char *path)
{
	size_t pos = 0;
	for (size_t i = 0; i < strlen(path); ++i)
	{
		if (path[i] == '/')
		{
			char *pre = strsub(path, 0, pos);
			char *sub = strsub(path, pos, i);

			if (sub)
			{
				const char *final_dir_path = pre ?
					formate_string("mkdir %s%s", pre, sub) :
					formate_string("mkdir %s", sub);

				if (!directory_exist(strsub(final_dir_path, 6, strlen(final_dir_path))))
					command_execute(final_dir_path);
			}

			pos = i + 1;
		}
	}

	char *pre = strsub(path, 0, pos);
	char *sub = strsub(path, pos, strlen(path));
	if (sub)
	{
		const char *final_dir_path = pre ?
			formate_string("mkdir %s%s", pre, sub) :
			formate_string("mkdir %s", sub);

		if (!directory_exist(strsub(final_dir_path, 6, strlen(final_dir_path))))
			command_execute(final_dir_path);
	}
}

time_t get_time_from_file(const char *path)
{
	struct stat file_stat;
	if (stat(path, &file_stat) == -1) {
		perror(formate_string("Failed to get file status: %s, ", path));
		return (time_t)(-1);  // Return -1 on error
	}

	return file_stat.st_mtime;
}

bool binary_test(const char *binary_path, array_T *source_list)
{
	time_t binary_timestamp = get_time_from_file(binary_path);
	if (binary_timestamp == (time_t)(-1))
		return true;

	for (size_t i = 0; i < source_list->index; ++i) {
		time_t source_timestamp = get_time_from_file(array_get(source_list, i));
		if (source_timestamp == (time_t)(-1)) {
			fprintf(
					stderr,
					"Failed to get modification time for source file: %s\n",
					array_get(source_list, i)
			);
			continue;
		}

		if (source_timestamp > binary_timestamp)
			return true;
	}

	return false;
}

hash_T *init_hash(void)
{
	hash_T *hash = arena_allocate(build_main_arena, sizeof(struct HASH_STRUCT));
	if (!hash)
		return NULL;

	hash->length = 0;
	hash->capacity = MAX_HASH_TABLE_CAPACITY;
	hash->buffer = arena_allocate(build_main_arena, sizeof(hash_entry) * hash->capacity);
	if (!hash->buffer)
		return NULL;

	return hash;
}

void hash_free(hash_T *hash)
{
	free(hash->keys);
	free(hash->buffer);
	free(hash);
}

void hash_set(hash_T *hash, const char *key, void *value)
{
	unsigned long long magic_number = hash_function(key);
	ssize_t index = (ssize_t)(magic_number & (MAX_HASH_TABLE_CAPACITY - 1));

	int if_found = 0;

	while (hash->buffer[index].key != NULL)
	{
		if (strcmp(key, hash->buffer[index].key) == 0)
		{
			hash->buffer[index].value = value;
			if_found = 1;
			break;
		}

		index++;
		if (index >= MAX_HASH_TABLE_CAPACITY) index = 0;
	}

	if (if_found == 0)
	{
		hash->buffer[index].key = (const char*)strdup(key);
		hash->buffer[index].value = value;
		hash->length++;

		if (hash->keys)
			hash->keys = realloc(hash->keys, sizeof(char*) * hash->length);
		else
			hash->keys = calloc(1, sizeof(char*));

		hash->keys[hash->length - 1] = (char*)strdup(key);
	}
}

void* hash_get(hash_T *hash, const char *key)
{
	unsigned long long magic_number = hash_function(key);
	ssize_t index = (ssize_t)(magic_number & (MAX_HASH_TABLE_CAPACITY - 1));

	while (hash->buffer[index].key != NULL)
	{
		if (strcmp(key, hash->buffer[index].key) == 0)
			return hash->buffer[index].value;

		index++;
		if (index >= MAX_HASH_TABLE_CAPACITY) index = 0;
	}
	
	return NULL;
}

const char **hash_bucket(hash_T *hash)
{
	return (const char**)hash->keys;
}

void exit_build_system(int status_code)
{
	if (array_allocated_counter > 0)
		WARN("You haven't freed %d array, which may result in memory leaks.", array_allocated_counter);

	free(build_main_arena->buffer);
	free(build_main_arena);

	exit(status_code);
}

// main entry
int main(int argc, char **argv)
{
	build_main_arena = init_arena(8 * 1024 * 1024); // allocating 8 mb, (hopefully it will be enough)
	
	const char *files_string = formate_string("%s build.h", build_source);
	array_T *files = strtoarray(files_string, ' ');

	if (binary_test(build_bin, files))
	{
		bool status = command_execute(formate_string("gcc %s -I. -o %s", build_source, build_bin));
		if (!status) INFO("sucessfully updated binary of build system. Re-run to see the changes.");
		else ERROR("failed to update the binary of build system.");

		return 0;
	}

	array_free(files);

	build_start(argc, argv);

	exit_build_system(0);
}

#endif // IMPLEMENT_BUILD_H
