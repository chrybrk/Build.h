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
 * Function: get_term_color(TERM_KIND kind, TERM_COLOR color)
 * -----------------------
 *  Generates a string for printing colored text.
 *
 * kind: Color affecting what part of text. (TERM_KIND)
 * color: Color of the text (TERM_COLOR)
 *
 * returns: Generated color format (const char *) 
 */
const char *get_term_color(TERM_KIND kind, TERM_COLOR color);

/*
 * Function: writef_function(char *s, ...)
 * -----------------------
 *  It works like `printf` but it returns formated string.
 *
 * s: Formated string (char *)
 * ...: Arguments (auto)
 *
 * returns: Formated string (char *) 
 *
 * Note: String is allocated in heap, so it must be freed.
 */
char *writef_function(char *s, ...);

/*
 * Function: substr(const char *string, size_t n1, size_t n2)
 * -----------------------
 *  It will create a sub-string from given string,
 *  in the range of n1 to n2.
 *
 * string: Initial string (const char *)
 * n1: Starting index in the string (size_t)
 * n2: Ending index in the string (size_t)
 *
 * returns: Sub-string (char *) 
 *
 * Note: String is allocated in heap, so it must be freed.
 */
char *substr(const char *string, size_t n1, size_t n2);

/*
 * Function: get_list_of_files(const char *path, int *count)
 * -----------------------
 *  Find all the files in the given path.
 *
 * path: Path (const char *)
 * count: Sets count to be the length of the buffer (int *)
 *
 * returns: String buffer (char **) 
 *
 * Note: String is allocated in heap, so it must be freed.
 */
char **get_list_of_files(const char *path, int *count);

/*
 * Function: get_last_modification_time(const char *filename)
 * -----------------------
 *  Returns the last modified time of the given file.
 *
 * filename: Path to the file (const char *)
 *
 * returns: Modified time of time (time_t) 
 */
time_t get_last_modification_time(const char *filename);

/*
 * Function: needs_recompilation(const char *binary, const char *sources[], size_t num_sources)
 * -----------------------
 *  Returns true if binary needs to be compiled,
 *  if any of the given source has been modified after the binary is created.
 *
 * binary: Path to binary file (const char *)
 * sources: List of source files that needs to check for modification (const char *[])
 * num_sources: Length of the list (size_t) 
 *
 * returns: Returns boolean if binary needs compilation (bool) 
 */
bool needs_recompilation(const char *binary, const char *sources[], size_t num_sources);

/*
 * Function: join(unsigned char sep, const char **buffer, size_t n)
 * -----------------------
 *  Join the list of string into one string and separate them by a seperator.
 *
 * binary: Separator (unsigned char)
 * buffer: List of string (const char **)
 * n: Length of the list (size_t) 
 *
 * returns: Returns new string separated by seperator (char *) 
 *
 * Note: String is allocated in the heap, so it must be freed.
 */
char *join(unsigned char sep, const char **buffer, size_t n);

/*
 * Function: separate(unsigned char sep, const char *string, size_t *n)
 * -----------------------
 *  Converts single string to list of multiple string from the seperator.
 *
 * seperator: Where to break the string (unsigned char)
 * string: String (const char *)
 * n: Length of list of strings (size_t *)
 *
 * returns: List of string (char **) 
 *
 * Note: String is allocated in the heap, so it must be freed.
 */
char **separate(unsigned char sep, const char *string, size_t *n);

/*
 * Function: cmd_execute(char *first, ...)
 * -----------------------
 *  Executes shell commands, using `system`.
 *
 * first: First command (char *)
 * ...: Rest of the commands (char *)
 *
 */
void cmd_execute(char *first, ...);

/*
 * Function: run_command(const char *command)
 * -----------------------
 *  Executes shell commands and returns output of the command.
 *
 * command: Command (const char *)
 *
 * returns: Returns output (char *) 
 *
 * Note: String is allocated in the heap, so it must be freed.
 */
char *run_command(const char *command);

/*
 * Function: strlistcmp(const char *s1, const char **s2, size_t n)
 * -----------------------
 *  It compares `s1` to `s2` and `s2` is a list of string.
 *
 * s1: String to compare from (const char *)
 * s2: String2 to compare with (const char **)
 * n: Length of list of strings `s2` (size_t)
 *
 * returns: True if `s1` matches with `s2` else False;
 *
 */
bool strlistcmp(const char *s1, const char **s2, size_t n);

/*
 * Function: is_directory_exists(const char *path)
 * -----------------------
 *  Checks if path is a directory or not.
 *
 * path: Path of the directory (const char *)
 *
 * returns: True if it founds the path to be directory; else false. 
 *
 */
bool is_directory_exists(const char *path);

/*
 * Function: create_directories(const char *s)
 * -----------------------
 *  Converts `s` to list of string,
 *  and checks if item of list are directory
 *  if they're not then it will create a new directory.
 *
 * s: Directories separated by a whitespace (const char *)
 *
 */
void create_directories(const char *s);

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

char **separate(unsigned char sep, const char *string, size_t *n)
{
	size_t len = 0;

	for (size_t i = 0; i < strlen(string); ++i)
		if (string[i] == sep) len++;

	*n = len;
	char **buffer = (char**)malloc(len * sizeof(char**));

	size_t p1 = 0;
	size_t p2 = 0;
	size_t b_idx = 0;

	for (; p2 < strlen(string); ++p2)
	{
		if (string[p2] == sep)
		{
			char *sub_str = substr(string, p1, p2);
			size_t sub_str_len = strlen(sub_str);

			buffer[b_idx] = (char*)malloc(sub_str_len * sizeof(char));
			strcpy(buffer[b_idx], sub_str);

			free(sub_str);
			p1 = p2 + 1;
			b_idx++;
		}
	}

	return buffer;
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

char *run_command(const char *command)
{
	char* result = NULL;
	size_t size = 0;
	FILE* fp = popen(command, "r");
	if (fp == NULL)
	{
		perror("popen failed");
		return NULL;
	}

	// Read the output a line at a time
	char buffer[128];
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		size_t len = strlen(buffer);
		char* new_result = realloc(result, size + len + 1);
		if (new_result == NULL)
		{
			perror("realloc failed");
			free(result);
			pclose(fp);
			return NULL;
		}
		result = new_result;
		memcpy(result + size, buffer, len);
		size += len;
		result[size] = '\0';
	}

	pclose(fp);
	return result;
}

bool strlistcmp(const char *s1, const char **s2, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		if (!strcmp(s1, s2[i])) return true;

	return false;
}

bool is_directory_exists(const char *path)
{
	char *t = run_command(writef("test -d %s && echo 0 || echo 1", path));
	return !atoi(t);
}

void create_directories(const char *s)
{
	size_t n;
	char **bf = separate(' ', s, &n);

	size_t len = 0;
	for (size_t i = 0; i < n; ++i)
	{
		len += strlen(bf[i]) + 1; // later this will be used for seprating last element.
		if (!is_directory_exists(bf[i]))
			CMD("mkdir", bf[i]);
	}

	// get last element from the string
	// because the string ends and it might not have whitespace
	// so it would not separate from the string.
	char *last_element = substr(s, len, strlen(s));

	if (!is_directory_exists(last_element))
		CMD("mkdir", last_element);
}

/*
 * build_itself()
 *
 * It is a function that gets called automatically,
 * it checks the status of current build source and build binary.
 * If it needs recompilition then it would do it.
*/
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

#endif // IMPLEMENT_BUILD_C
