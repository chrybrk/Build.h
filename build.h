#ifdef IMPLEMENT_BUILD_C

#if __WIN32__
#include <errno.h>
#endif

// ------------------------------------------

// Regular text
#define TBLK "\e[0;30m"
#define TRED "\e[0;31m"
#define TGRN "\e[0;32m"
#define TYEL "\e[0;33m"
#define TBLU "\e[0;34m"
#define TMAG "\e[0;35m"
#define TCYN "\e[0;36m"
#define TWHT "\e[0;37m"

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

// Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

// Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

// High intensty background 
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

// High intensty text
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"

// Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"

// Reset
#define reset "\e[0m"
#define CRESET "\e[0m"
#define COLOR_RESET "\e[0m"

// ------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#if __UNIX__
#include <sys/wait.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#ifndef SOURCE_C_FILE
#define SOURCE_C_FILE "build.c"
#endif

#ifndef SOURCE_EXEC_FILE
#if __UNIX__
#define SOURCE_EXEC_FILE "build"
#elif __WIN32__
#define SOURCE_EXEC_FILE "build.exe"
#endif
#endif

#define safe(value)	__safe_callback__(value, (char*)__FILE__, __LINE__, (char*)__FUNCTION__)
#define INFO(...)	printf("%s[INFO]:%s %s\n", BGRN, reset, writef(__VA_ARGS__));
#define WARN(...)	printf("%s[WARN]:%s %s\n", BYEL, reset, writef(__VA_ARGS__));
#define ERROR(...) printf("%s[ERROR]:%s %s\n", BRED, reset, writef(__VA_ARGS__)), exit(1);
#define ACTION(...) printf("%s[ACTION]:%s %s\n", BCYN, reset, writef(__VA_ARGS__));
#define CMD(...) cmd_execute(__VA_ARGS__, NULL)
#define writef(...) ({  writef_function(__VA_ARGS__, NULL); })

bool state_of_rebuilds = false;

char *writef_function(char *s, ...);
void cmd_execute(char *first, ...);
char *join(unsigned char sep, const char **buffer, size_t n);
void *__safe_callback__(void *PTR, char *F, int LINE, char *FUNCTION_NAME);
time_t get_last_modification_time(const char *filename);
bool needs_recompilation(const char *binary, const char *sources[], size_t num_sources);
char **get_files_from_directory(const char *path, int *count);

char **get_files_from_directory(const char *path, int *count)
{
	int internalCounter = 0;
	char **buffer = (char**)safe(malloc(sizeof(char**) * internalCounter));

	DIR *dir = opendir(path);
	if (dir == NULL) return NULL;

	struct dirent *data;
	while ((data = readdir(dir)) != NULL)
	{
		// it means that the current *ptr is a file not a directory
#		if __UNIX__
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
	{
		state_of_rebuilds = true;
		return true;
	}

	for (size_t i = 0; i < num_sources; ++i) {
		time_t source_timestamp = get_last_modification_time(sources[i]);
		if (source_timestamp == (time_t)(-1)) {
			fprintf(stderr, "Failed to get modification time for source file: %s\n", sources[i]);
			continue;
		}

		if (source_timestamp > binary_timestamp)
		{
			state_of_rebuilds = true;
			return true;
		}
	}

	return false;
}

char *writef_function(char *s, ...)
{
	size_t buffer_size = 64; // 64 bytes
	char *buffer = (char*)malloc(buffer_size);

	if (buffer == NULL) return NULL;

	va_list ap;
	va_start(ap, s);

	int nSize = vsnprintf(buffer, buffer_size, s, ap);
	if (nSize < 0)
	{
		free(buffer);
		va_end(ap);
	}

	if (nSize >= buffer_size)
	{
		buffer_size = nSize + 1;
		buffer = (char*)realloc(buffer, buffer_size);

		if (buffer == NULL) return NULL;

		va_end(ap);

		va_start(ap, s);
		vsnprintf(buffer, buffer_size, s, ap);
	}

	va_end(ap);

	return buffer;
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

	return bf;
}

void *__safe_callback__(void *PTR, char *F, int LINE, char *FUNCTION_NAME)
{
	if (PTR == NULL)
	{
		fprintf(stdout, "%s:%d :: In %s: \n\tExpected not NULL value, but got NULL.\n", F, LINE, FUNCTION_NAME);
		exit(EXIT_FAILURE);
	}
	
	return PTR;
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
	safe(buffer);

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
	INFO("CMD: %s", b);

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
	const char *sources[] = { SOURCE_C_FILE, "build.h" };
	if (needs_recompilation(SOURCE_EXEC_FILE, sources, sizeof(sources) / sizeof(sources[0])))
	{
		INFO("Source file has changed, it needs to be recompiled.");
		CMD("gcc", SOURCE_C_FILE, "-I.", "-o", writef("%s.new", SOURCE_EXEC_FILE));
		CMD("mv", SOURCE_EXEC_FILE, writef("%s.old", SOURCE_EXEC_FILE));
		CMD("mv", writef("%s.new", SOURCE_EXEC_FILE), SOURCE_EXEC_FILE);
		exit(0);
	}
}

#endif