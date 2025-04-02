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

#ifdef IMPLEMENT_BUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
#define ERROR(...) printf("%s[BUILD :: ERROR]:%s %s\n", get_term_color(TEXT, RED), get_term_color(RESET, 0), formate_string(__VA_ARGS__)), exit(1);

#define LOAD_FACTOR 0.6
#define POWER_FACTOR 1.5

#define darray_push(array, item) { 																																	\
	__dynamic_array_t *meta = __darray_get_meta__(array); 																					\
	if (((float)meta->index / (float)meta->count) >= LOAD_FACTOR) { 																	\
		meta->count *= POWER_FACTOR; 																																		\
		array = __darray_get_meta__(array); 																																\
		array = realloc(array, sizeof(__dynamic_array_t) + meta->count * sizeof(typeof(item))); 		\
		meta = (__dynamic_array_t*)array; 																													\
		array = __darray_get_array__(array); 																															\
	} 																																																\
	array[meta->index++] = item; 																																			\
}
#define darray_free(array) free(__darray_get_meta__(array))
#define darray_get_length(array) (((__dynamic_array_t*)__darray_get_meta__(array))->index)

#define __hm_realloc__(KVs, KV) { \
	__hashmap_t *meta = __hashmap_get_meta__(KVs); \
	if (((float)meta->index / (float)meta->count) >= LOAD_FACTOR) { \
		meta->count *= POWER_FACTOR; \
		struct bucket *buckets = malloc(sizeof(struct bucket) * meta->count); \
		for (size_t i = 0; i < meta->count / POWER_FACTOR; ++i) { \
			struct bucket bkt = meta->buckets[i]; \
			if (!bkt.filled) continue; \
			uint64_t index = meta->hf(KVs[bkt.index].key, bkt.key_size) % meta->count; \
			uint64_t c = 1; \
			while (buckets[index].filled) \
			{\
				c++; \
				index = (meta->hf(KVs[bkt.index].key, bkt.key_size) ^ c) % meta->count; \
			}\
			buckets[index].key_size = bkt.key_size; \
			buckets[index].index = bkt.index; \
			buckets[index].filled = 1; \
		} \
		KVs = __hashmap_get_meta__(KVs); \
		KVs = realloc(KVs, sizeof(__hashmap_t) + meta->count * sizeof(typeof(KV))); \
		meta = (__hashmap_t*)KVs; \
		meta->buckets = realloc(meta->buckets, sizeof(struct bucket) * meta->count); \
		memcpy(meta->buckets, buckets, sizeof(struct bucket) * meta->count); \
		KVs = __hashmap_get_KVs__(KVs); \
	} \
}

#define hm_put(KVs, KV, iKS) { \
	__hm_realloc__(KVs, KV); \
	__hashmap_t *meta = __hashmap_get_meta__(KVs); \
	uint64_t index = meta->hf(KV.key, iKS) % meta->count; \
	uint64_t c = 0; \
	while (c < meta->count) \
	{\
		index = (index + c * c) % meta->count; \
		if (iKS == meta->buckets[index].key_size && meta->hc(KV.key, KVs[meta->buckets[index].index].key, iKS)) {\
			KVs[meta->buckets[index].index] = KV; \
			break; \
		}\
		if (!meta->buckets[index].filled) {\
			meta->buckets[index].key_size = iKS; \
			meta->buckets[index].index = meta->index; \
			meta->buckets[index].filled = 1; \
			KVs[meta->index++] = KV; \
			break; \
		}\
		c++; \
	}\
}

#define hm_get(KVs, KV, iKS) { \
	__hashmap_t *meta = __hashmap_get_meta__(KVs); \
	uint64_t index = meta->hf(KV->key, iKS) % meta->count; \
	uint64_t c = 0; \
	while (c < meta->count) \
	{\
		index = (index + c * c) % meta->count; \
		if (iKS == meta->buckets[index].key_size && meta->hc(KV->key, KVs[meta->buckets[index].index].key, iKS)) {\
			KV->value = KVs[meta->buckets[index].index].value; \
			break; \
		}\
		c++; \
	}\
}

#define hm_free(KVs) free(__hashmap_get_meta__(KVs))
#define hm_get_length(KVs) (((__hashmap_t*)__hashmap_get_meta__(KVs))->index)

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

typedef struct {
	size_t count;
	size_t index;
} __dynamic_array_t;

typedef uint64_t (*hash_function_t)(void *bytes, size_t size);
typedef int (*hash_cmp_t)(void *a, void *b, size_t size);

typedef struct {
	hash_function_t hf;
	hash_cmp_t hc;
	struct bucket {
		size_t key_size;
		size_t index;
		uint8_t filled;
	} *buckets;
	size_t count;
	size_t index;
} __hashmap_t;

void *__darray_get_meta__(void *array);
void *__darray_get_array__(void *array);
void *__hashmap_get_meta__(void *KVs);
void *__hashmap_get_KVs__(void *KVs);

// build files
extern const char *build_source;
extern const char *build_bin;

// color for logging
const char *get_term_color(TERM_KIND kind, TERM_COLOR color);

// macro-related function for formating string
char *__formate_string_function__(char *s, ...);

// string join
char *join_string(const char *s0, const char *s1);

// string sub-string
char *sub_string(const char *s, size_t fp, size_t tp);

// string replace
char *replace_char_in_string(char *s, unsigned char from, unsigned char to);

// convert (const char **) to (const char *)
const char *string_list_to_const_string(const char **string_list, size_t len, unsigned char sep);

// convert const string to array with seprator
const char **string_to_array(const char *string, unsigned char sep);

// exectue command in the shell
bool execute(const char *command);

// check if binary needs recompilation
bool is_binary_old(const char *bin_path, const char **array);

// check if file exist
bool is_file_exists(const char *path);

// check if directory exist
bool is_directory_exists(const char *path);

// create new directory (it will create all directory in the path)
void create_directory(const char *path);

// get files in array
const char **get_files(const char *from);

// get files with specific extention
const char **get_files_with_specific_ext(const char *from, const char *ext);

// convert string list to array
const char **string_list_to_array(const char **string_list, size_t len);

// read entire file
const char *read_file(const char *path);

// write to file
void write_file(const char *path, const char *buffer);

// get last modified time of file
time_t get_time_from_file(const char *path);

// init dynamic array
void *init_darray(void *array, size_t item_size, size_t inital_size);

// init hashmap
void *init_hm(void *KVs, size_t KV_size, size_t inital_capacity, hash_function_t hf, hash_cmp_t hc);

// fnv-1a hash function
uint64_t fnv_1a_hash(void *bytes, size_t size);

// memory compare for two pointers
int cmp_hash(void *a, void *b, size_t size);

/**********************************************************************************************
*																   The Actual Implementation
**********************************************************************************************/

const char *get_term_color(TERM_KIND kind, TERM_COLOR color)
{
	switch (kind)
	{
		case TEXT: return formate_string((char *)"\e[0;3%dm", color);
		case BOLD_TEXT: return formate_string((char *)"\e[1;3%dm", color);
		case UNDERLINE_TEXT: return formate_string((char *)"\e[4;3%dm", color);
		case BACKGROUND: return formate_string((char *)"\e[4%dm", color);
		case HIGH_INTEN_BG: return formate_string((char *)"\e[0;10%dm", color);
		case HIGH_INTEN_TEXT: return formate_string((char *)"\e[0;9%dm", color);
		case BOLD_HIGH_INTEN_TEXT: return formate_string((char *)"\e[1;9%dm", color);
		case RESET: return formate_string((char *)"\e[0m");
	}

	return "";
}

char *__formate_string_function__(char *s, ...)
{
	// allocate small size buffer
	size_t buffer_size = 64; // bytes
	char *buffer = (char *)malloc(buffer_size);

	if (buffer == NULL)
	{
		perror("failed to allocate memory for formating string: ");
		return NULL;
	}

	va_list ap;
	va_start(ap, s);

	size_t nSize = vsnprintf(buffer, buffer_size, s, ap);
	if (!nSize)
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

char *join_string(const char *s0, const char *s1)
{
	char *s = (char *)malloc(strlen(s0) + strlen(s1) + 1);

	s = strcpy(s, s0);
	s = strcat(s, s1);

	return s;
}

char *sub_string(const char *s, size_t fp, size_t tp)
{
	if (fp >= tp) return NULL;

	char *sub = (char *)malloc((tp - fp) + 1);
	for (size_t i = 0; i < tp - fp; ++i)
		sub[i] = s[fp + i];

	sub[(tp - fp)] = '\0';
	
	return sub;
}

char *replace_char_in_string(char *s, unsigned char from, unsigned char to)
{
	for (size_t i = 0; i < strlen(s); ++i)
		if (s[i] == from)
			s[i] = to;

	return s;
}

const char *string_list_to_const_string(const char **string_list, size_t len, unsigned char sep)
{
	size_t total_size_of_string = 0;

	for (size_t i = 0; i < len; ++i)
		total_size_of_string += strlen(string_list[i]) + 1;

	char *string = (char *)malloc(total_size_of_string + 1);
	size_t pos = 0;
	for (size_t i = 0; i < len; ++i)
	{
		string = strcat(string, string_list[i]);
		pos += strlen(string_list[i]) + 1;
		string[pos - 1] = sep;
	}

	string[total_size_of_string] = '\0';

	return string;
}

const char **string_to_array(const char *string, unsigned char sep)
{
	const char **array = NULL;
	array = init_darray(array, 8, 32);

	size_t pos = 0;
	for (size_t i = 0; i < strlen(string); ++i)
	{
		if (string[i] == sep)
		{
			char *nstr = sub_string(string, pos, i);
			darray_push(array, nstr);
			pos = i + 1;
		}
	}

	char *nstr = sub_string(string, pos, strlen(string));
	darray_push(array, nstr);

	return array;
}

bool execute(const char *command)
{
	if (command == NULL) return false;
	INFO("$ %s", command);
	return system(command);
}

bool is_binary_old(const char *bin_path, const char **array)
{
	time_t binary_timestamp = get_time_from_file(bin_path);
	if (binary_timestamp == (time_t)(-1))
		return true;

	for (size_t i = 0; i < darray_get_length(array); ++i) {
		time_t source_timestamp = get_time_from_file(array[i]);
		if (source_timestamp == (time_t)(-1)) {
			fprintf(
					stderr,
					"Failed to get modification time for source file: %s\n",
					array[i]
			);
			continue;
		}

		if (source_timestamp > binary_timestamp)
			return true;
	}

	return false;
}

bool is_file_exists(const char *path)
{
	FILE *file = fopen(path, "r");
	if (file == NULL) return false;

	fclose(file);

	return true;
}

bool is_directory_exists(const char *path)
{
	DIR *dir = opendir(path);
	if (dir == NULL) return false;

	closedir(dir);
	return true;
}

void create_directory(const char *path)
{
	size_t pos = 0;
	for (size_t i = 0; i < strlen(path); ++i)
	{
		if (path[i] == '/')
		{
			char *pre = sub_string(path, 0, pos);
			char *sub = sub_string(path, pos, i);

			if (sub)
			{
				const char *final_dir_path = pre ?
					formate_string((char *)"mkdir %s%s", pre, sub) :
					formate_string((char *)"mkdir %s", sub);

				if (!is_directory_exists(sub_string(final_dir_path, 6, strlen(final_dir_path))))
					execute(final_dir_path);
			}

			pos = i + 1;
		}
	}

	char *pre = sub_string(path, 0, pos);
	char *sub = sub_string(path, pos, strlen(path));
	if (sub)
	{
		const char *final_dir_path = pre ?
			formate_string((char *)"mkdir %s%s", pre, sub) :
			formate_string((char *)"mkdir %s", sub);

		if (!is_directory_exists(sub_string(final_dir_path, 6, strlen(final_dir_path))))
			execute(final_dir_path);
	}
}

// get files in array
const char **get_files(const char *from)
{
	bool hash_slash = false;
	char slash = sub_string(from, strlen(from) - 1, strlen(from))[0];

	if (slash == '/' || slash == '\\')
		hash_slash = true;

	DIR *dir = opendir(from);
	if (dir == NULL)
	{
		fprintf(stderr, formate_string((char *)"Directory `%s` does not exist.", from));
		return NULL;
	}

	char **buffer = NULL;
	buffer = (char**)init_darray(buffer, 8, 32);

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
			char *item = NULL;
			if (hash_slash)
				item = formate_string((char *)"%s%s", from, fileName);
			else
#				if __unix__
					item = formate_string((char *)"%s/%s", from, fileName);
#				elif __WIN32__
					item = formate_string((char *)"%s\\%s", from, fileName);
#				endif
			darray_push(buffer, item);
		}
	}

	return (const char **)buffer;
}

// get files with specific extention
const char **get_files_with_specific_ext(const char *from, const char *ext)
{
	const char **files = get_files(from);
	const char **files_with_excep = NULL;
	files_with_excep = (const char **)init_darray(files_with_excep, 8, 32);

	for (size_t i = 0; i < darray_get_length(files); ++i)
	{
		char *file = (char *)files[i];
		char *substr = sub_string(file, strlen(file) - strlen(ext), strlen(file));

		if (!strcmp(substr, ext))
			darray_push(files_with_excep, file);
	}

	darray_free(files);
	return files_with_excep;
}

// convert string list to array
const char **string_list_to_array(const char **string_list, size_t len)
{
	const char **array = NULL;
	array = (const char **)init_darray(array, 8, len);

	for (size_t i = 0; i < len; ++i)
		darray_push(array, string_list[i]);

	return array;
}

// read entire file
const char *read_file(const char *path)
{
	FILE *fp;
	char *line = NULL;
	long len;

	fp = fopen(path, "rb");
	if ( fp == NULL )
	{
		ERROR("Cannot read file from this filepath, `%s`\n", path);
		return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char *buffer = (char*)calloc(len, sizeof(char));
	if (buffer == NULL)
			return NULL;

	fread(buffer, sizeof(char), len - 1, fp);

	fclose(fp);
	if (line)
			free(line);

	return buffer;
}

// write to file
void write_file(const char *path, const char *buffer)
{
	FILE * fp;

	fp = fopen(path, "w");
	if ( fp == NULL )
	{
		ERROR("Cannot read file from this filepath, `%s`\n", path);
		return;
	}

	fputs(buffer, fp);
	fclose(fp);
}

// get last modified time of file
time_t get_time_from_file(const char *path)
{
	struct stat file_stat;
	if (stat(path, &file_stat) == -1) {
		perror(formate_string("Failed to get file status: %s, ", path));
		return (time_t)(-1);  // Return -1 on error
	}

	return file_stat.st_mtime;
}

// init dynamic array
void *init_darray(void *array, size_t item_size, size_t inital_size)
{
	array = malloc(sizeof(__dynamic_array_t) + inital_size * item_size);
	((__dynamic_array_t*)array)->count = inital_size;
	((__dynamic_array_t*)array)->index = 0;

	return __darray_get_array__(array);
}

// init hashmap
void *init_hm(void *KVs, size_t KV_size, size_t inital_capacity, hash_function_t hf, hash_cmp_t hc)
{
	KVs = malloc(sizeof(__hashmap_t) + inital_capacity * KV_size);
	((__hashmap_t*)KVs)->buckets = malloc(inital_capacity * sizeof(struct bucket));
	((__hashmap_t*)KVs)->count = inital_capacity;
	((__hashmap_t*)KVs)->index = 0;
	((__hashmap_t*)KVs)->hf = hf;
	((__hashmap_t*)KVs)->hc = hc;

	return __hashmap_get_KVs__(KVs);
}

// fnv-1a hash function
uint64_t fnv_1a_hash(void *bytes, size_t size)
{
	uint64_t h = 14695981039346656037ULL; // FNV-1a hash
	for (size_t i = 0; i < size; ++i) {
		h ^= ((unsigned char*)bytes)[i];
		h *= 1099511628211ULL; // FNV prime
	}
	return h;
}

// memory compare for two pointers
int cmp_hash(void *a, void *b, size_t size)
{
	return memcmp(a, b, size) == 0;
}

void *__darray_get_meta__(void *array)
{
	return array - (offsetof(__dynamic_array_t, index) + sizeof(((__dynamic_array_t*)0)->index));
}

void *__darray_get_array__(void *array)
{
	return array + (offsetof(__dynamic_array_t, index) + sizeof(((__dynamic_array_t*)0)->index));
}

void *__hashmap_get_meta__(void *KVs)
{
	return KVs - (offsetof(__hashmap_t, index) + sizeof(((__hashmap_t*)0)->index));
}

void *__hashmap_get_KVs__(void *KVs)
{
	return KVs + (offsetof(__hashmap_t, index) + sizeof(((__hashmap_t*)0)->index));
}

void build_itself() __attribute__((constructor));
void build_itself()
{
	const char **build_files = string_list_to_array((const char *[]){ build_source, "build.h" }, 2);
	if (is_binary_old(build_bin, build_files))
	{
		execute(formate_string("cc -o %s %s", build_bin, build_source));
		darray_free(build_files);
		exit(0);
	}
}

#endif // IMPLEMENT_BUILD_H
