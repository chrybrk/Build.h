#ifdef IMPLEMENT_BUILD_C

// ------------------------------------------

// Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

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
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#ifndef SOURCE_C_FILE
#define SOURCE_C_FILE "build.c"
#endif

#ifndef SOURCE_EXEC_FILE
#define SOURCE_EXEC_FILE "build"
#endif

#define safe(value)	__safe_callback__(value, (char*)__FILE__, __LINE__, (char*)__FUNCTION__)
#define INFO(...)	printf("%s[INFO]:%s %s\n", BGRN, reset, writef(__VA_ARGS__));
#define WARN(...)	printf("%s[WARN]:%s %s\n", BYEL, reset, writef(__VA_ARGS__));
#define ERROR(...) printf("%s[ERROR]:%s %s\n", BRED, reset, writef(__VA_ARGS__)), exit(1);
#define ACTION(...) printf("%s[ACTION]:%s %s\n", BCYN, reset, writef(__VA_ARGS__));
#define CMD(...) cmd_execute(__VA_ARGS__, NULL)

bool state_of_rebuilds = false;

char *writef(char *s, ...);
void cmd_execute(char *first, ...);
char *join(unsigned char sep, char **buffer, size_t n);
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
		if (data->d_type != DT_DIR)
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

char *writef(char *s, ...)
{
    char *string;
    va_list args;
    va_start(args, s);
    vasprintf(&string, s, args);
    va_end(args);
    return string;
}

char *join(unsigned char sep, char **buffer, size_t n)
{
	char *localBuffer = NULL;

	for (unsigned int i = 0; i < n; ++i)
	{
		const char *string = writef("%s%c", buffer[i], sep);

		if (localBuffer)
		{
			localBuffer = realloc(localBuffer, (strlen(string) + strlen(localBuffer)) * sizeof(char) + 8);
			localBuffer = strcat(localBuffer, string);
		}
		else
		{
			localBuffer = malloc(strlen(string) * sizeof(char) + 8);
			localBuffer = strcpy(localBuffer, string);
		}

		localBuffer[strlen(localBuffer)] = '\0';
	}

	return localBuffer;
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

	char *buffer[length];
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

	char *b = join(' ', buffer, length);
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
		CMD("cc", SOURCE_C_FILE, "-I.", "-o", SOURCE_EXEC_FILE);
		CMD(writef("./%s", SOURCE_EXEC_FILE));
		exit(0);
	}
}

#endif
