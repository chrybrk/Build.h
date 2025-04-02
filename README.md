# Build.h

`build.h` is a single-header library that acts as a build system, it works more-or-less like shell-script, with many built-in features:

- `Execute`
- `INFO`
- `WARN`
- `ERROR`
- `formate_string`
- `substr`
- `get_list_of_files`
- `get_list_of_files_ext`
- `join`
- `join_cstr`
- `seprate`
- `run_command`
- `strlistcmp`
- `is_directory_exists`
- `is_file_exists`

## Logging

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
	INFO("Information");
	WARN("Warnings");
	ERROR("Error & Exit");

	return 0;
}
```

## Execute commands

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    CMD("ls", "-l");
    CMD("pwd");

    // .. more shell commands

	return 0;
}
```

## writef (It works like printf, but returns a formated string)

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    const char *some_stuff = "-l";
    CMD(writef("ls %s", some_stuff));

	return 0;
}
```

## Substring in C

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    const char *string = "Hello, World!";
    const char *sub_string = substr(string, 0, 5);

	return 0;
}
```

## Join in C

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    const char **strings = { "no", "build" };
    char *joined_string = join(' ', strings, 2);

	return 0;
}
```

## Get files from directory

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    int n;
    char **files = get_list_of_files("./", &n);

    for (int i = 0; i < n; ++i)
        printf("%s\n", files[i]);

	return 0;
}
```

## Check if a binary need recompilation

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    int n;
    char **files = get_list_of_files("path/to/source", &n);

    bool recompilation = needs_recompilation("path/to/binary", (const char**)files, n);

	return 0;
}
```

## Download using curl, and use tar to un-archive

```c
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(void)
{
    struct download_info info = {
        .url = "some/url",
        .out_dir = "path/to/out",
        .filename = "outname",
        .extract = true,
        .extract_in_dir = "path/to/extract",
        .tar_command = "tar xf "
    };

    download(1, (struct download_info[]) { info });

    // .. add more to download more

	return 0;
}
```
