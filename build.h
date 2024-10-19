#ifdef IMPLEMENT_BUILD_C

/*
	MIT License

	Copyright (c) 2024 Chry003

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#if __WIN32__
	#include <errno.h>
#endif

#if __unix__
	#include <sys/wait.h>
#endif

// If user has not define file for auto compilation
#ifndef BUILD_SOURCE_FILE
	#define BUILD_SOURCE_FILE "build.c"
#endif // BUILD_SOURCE_FILE

#ifndef BUILD_OUTPUT_FILE
	#if __WIN32__
		#define BUILD_OUTPUT_FILE "build.exe" // windows
	#else
		#define BUILD_OUTPUT_FILE "build"			// linux
	#endif
#endif // BUILD_OUTPUT_FILE

#ifndef CMD_DEBUG_OUTPUT
	#define CMD_DEBUG_OUTPUT true
#endif // CMD_DEBUG_OUTPUT

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


/********************************************
 * 						MACRO FUNCTIONS	
********************************************/
#define CMD(...) cmd_execute(__VA_ARGS__, NULL)
#define writef(...) ({  writef_function(__VA_ARGS__, NULL); })
#define INFO(...) printf("%s[INFO]:%s %s\n", get_term_color(TEXT, GREEN), get_term_color(RESET, 0), writef(__VA_ARGS__))
#define WARN(...) printf("%s[WARN]:%s %s\n", get_term_color(TEXT, YELLOW), get_term_color(RESET, 0), writef(__VA_ARGS__))
#define ERROR(...) printf("%s[ERROR]:%s %s\n", get_term_color(TEXT, RED), get_term_color(RESET, 0), writef(__VA_ARGS__)), exit(1);

/********************************************
 * 							DECLARATION
********************************************/

/*
 * get_term_color(TERM_KIND, TERM_COLOR)
 * return terminal color string
*/
const char *get_term_color(TERM_KIND, TERM_COLOR);

/*
 * writef_function(char *, args) 
 * It works like printf but it returns string.
*/
char *writef_function(char *, ...);

/*
 * substr(const char*, size_t, size_t)
 * It will return sub string from point A to B
*/
char *substr(const char *, size_t, size_t);

/*
 * get_list_of_files(const char *, int *)
 * It will get all of the files in the given path,
 * (int *) will have the length of the given files.
 * (char **) return files;
*/
char **get_list_of_files(const char *, int *);

/*
 * get_files_from_directory(const char *)
 * last modified time of the given file.
*/
time_t get_last_modification_time(const char *);

/*
 * needs_recompilation(const char *, const char *[], size_t)
 * It will return true if any of the given files
 * have modification time greater than the binary.
*/
bool needs_recompilation(const char *, const char *[], size_t);

/*
 * join(unsigned char, const char **, size_t)
 * Adds seperator to the given string list,
 * and returns one whole string.
*/
char *join(unsigned char, const char **, size_t);

/*
 * cmd_execute(char *, args)
 * I think the function name is self-explanatory,
 * it lets you excute commands.
*/
void cmd_execute(char *, ...);

/*
 * strlistcmp(const char *, const char **, size_t)
 * like `strcmp` it compares two string together,
 * `strlistcmp` will compare multiple string with one
 * return true if any of the given files match.
*/
bool strlistcmp(const char *, const char **, size_t);

/********************************************
 * 						   DEFINITION	
********************************************/
const char *get_term_color(TERM_KIND kind, TERM_COLOR color)
{
	switch (kind)
	{
		case TEXT: return writef("\e[0;3%dm", color);
		case BOLD_TEXT: return writef("\e[1;3%dm", color);
		case UNDERLINE_TEXT: return writef("\e[4;3%dm", color);
		case BACKGROUND: return writef("\e[4%dm", color);
		case HIGH_INTEN_BG: return writef("\e[0;10%dm", color);
		case HIGH_INTEN_TEXT: return writef("\e[0;9%dm", color);
		case BOLD_HIGH_INTEN_TEXT: return writef("\e[1;9%dm", color);
		case RESET: return writef("\e[0m");
	}
}

char *writef_function(char *s, ...)
{
	// allocate small size buffer
	size_t buffer_size = 64; // 64 bytes
	char *buffer = (char*)malloc(buffer_size);

	if (buffer == NULL)
	{
		WARN("writef: Failed to allocate buffer.");
		return NULL;
	}

	va_list ap;
	va_start(ap, s);

	int nSize = vsnprintf(buffer, buffer_size, s, ap);
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
			WARN("writef: Failed to re-allocate buffer.");
			return NULL;
		}

		va_end(ap);

		va_start(ap, s);
		vsnprintf(buffer, buffer_size, s, ap);
	}

	va_end(ap);

	return buffer;
}

char *substr(const char *string, size_t n1, size_t n2)
{
	if (string == NULL)
	{
		WARN("substr: Cannot create substr from NULL.");
		return NULL;
	}

	size_t len = strlen(string);

	/*
	 * n1 and n2 must be greater than 0,
	 * and n1 must be smaller than n2,
	 * n2 must be smaller than total length.
	 *
	 * Otherwise return NULL;
	 */
	if (n1 < 0 && n2 < 0 && n1 >= n2 && n2 >= len)
	{
		WARN("substr: Undefined behaviour of `n1` and `n2`.");
		return NULL;
	}

	char *result = (char*)malloc((n2 - n1) * sizeof(char));
	if (result == NULL)
	{
		WARN("substr: Failed to allocate buffer.");
		return NULL;
	}

	for (size_t i = 0; i < n2 - n1; ++i)
		result[i] = string[i + n1];

	result[n2 - n1] = '\0';

	return result;
}

char **get_list_of_files(const char *path, int *count)
{
	int internalCounter = 0;
	char **buffer = (char**)malloc(sizeof(char**) * internalCounter);

	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		ERROR("Directory `%s` does not exist.", path);
		return NULL;
	}

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

			internalCounter++;
			buffer = (char**)realloc(buffer, internalCounter * sizeof(char**));

			buffer[internalCounter - 1] = writef("%s%s", path, fileName);
		}
	}

	*count = internalCounter;
	return buffer;
}

time_t get_last_modification_time(const char *filename)
{
	struct stat file_stat;
	if (stat(filename, &file_stat) == -1) {
		perror(writef("Failed to get file status: %s, ", filename));
		return (time_t)(-1);  // Return -1 on error
	}

	return file_stat.st_mtime;
}

bool needs_recompilation(const char *binary, const char *sources[], size_t num_sources)
{
	time_t binary_timestamp = get_last_modification_time(binary);
	if (binary_timestamp == (time_t)(-1))
		return true;

	for (size_t i = 0; i < num_sources; ++i) {
		time_t source_timestamp = get_last_modification_time(sources[i]);
		if (source_timestamp == (time_t)(-1)) {
			fprintf(stderr, "Failed to get modification time for source file: %s\n", sources[i]);
			continue;
		}

		if (source_timestamp > binary_timestamp)
			return true;
	}

	INFO("`%s` is already updated.", binary);

	return false;
}

char *join(unsigned char sep, const char **buffer, size_t n)
{
	/* *** BUFFER ALLOCATOR *** */

	/*
		* How this is going to work?
		* Well, we define a *pool, and fill it up.
		* If pools fills up, we re-alloc it with the same size.
	*/

	// set the max size for pool
	const size_t POOL_SIZE = 64 * 1024;

	char *pool = (char*)malloc(POOL_SIZE); // 65536 bytes
	if (pool == NULL) printf("Failed to create pool for joining text.\n");

	// set pool to NULL
	pool = (char*)memset(pool, 0, POOL_SIZE);

	size_t current_pool_size = POOL_SIZE;
	size_t ptr_in_pool = 0; // it will keep track where we are in the pool.	

	for (size_t i = 0; i < n; ++i)
	{
		const char *string = writef("%s%c", buffer[i], sep);
		size_t len = strlen(string);
		
		if ((ptr_in_pool + len) >= current_pool_size)
		{
			current_pool_size += POOL_SIZE;
			pool = (char*)realloc(pool, current_pool_size);
		}

		ptr_in_pool += len;
		pool = strcat(pool, string);
		pool[ptr_in_pool + 1] = '\0';
	}

	// create a final buffer to be returned.
	char *bf = (char*)malloc(ptr_in_pool);
	bf = strncpy(bf, pool, ptr_in_pool);
	bf[ptr_in_pool] = '\0'; // don't forget to add null ptr, it is pain in the ass.
	
	// we don't need pool because we have already created smaller pool of content size.
	free(pool);

	return bf;
}

void cmd_execute(char *first, ...)
{
	int length = 0;

	if (first == NULL)
		ERROR("No arguments given to CMD, exiting.");

	va_list args;
	va_start(args, first);
	for (
			char *next = va_arg(args, char*);
			next != NULL;
			next = va_arg(args, char*)) {
		length++;
	}
	va_end(args);

	char *buffer[length + 1];

	length = 0;
	buffer[length++] = first;

    va_start(args, first);
	for (
			char *next = va_arg(args, char*);
			next != NULL;
			next = va_arg(args, char*)) {
		buffer[length++] = next;
	}
	va_end(args);
	
	char *b = join(' ', (const char**)buffer, length);

#if CMD_DEBUG_OUTPUT
	INFO("CMD: %s", b);
#endif

	int status = system(b);
	if (status != 0)
	{
		ERROR("Failed: %s", b);
		exit(EXIT_FAILURE);
		free(b);
	}

	free(b);
}

void build_itself() __attribute__((constructor));
void build_itself()
{
	const char *sources[] = { BUILD_SOURCE_FILE, "build.h" };
	if (needs_recompilation(BUILD_OUTPUT_FILE, sources, sizeof(sources) / sizeof(sources[0])))
	{
		INFO("Source file has changed, it needs to be recompiled.");
		CMD("gcc", BUILD_SOURCE_FILE, "-I.", "-o", writef("%s.new", BUILD_OUTPUT_FILE));
		CMD("mv", BUILD_OUTPUT_FILE, writef("%s.old", BUILD_OUTPUT_FILE));
		CMD("mv", writef("%s.new", BUILD_OUTPUT_FILE), BUILD_OUTPUT_FILE);
#ifdef __unix__
		CMD(writef("./%s", BUILD_OUTPUT_FILE));
#endif
		exit(0);
	}
}

bool strlistcmp(const char *s1, const char **s2, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		if (!strcmp(s1, s2[i])) return true;

	return false;
}

#endif // IMPLEMENT_BUILD_C
