# Build.h

It is a header-only file which has few things,
1. You can execute commands.
2. Log something.
3. Check if a binary needs a recompilation.
4. Build system will build itself.
5. You can get files from directory (which is handy in some cases).

# How to use it?
```C
#define IMPLEMENT_BUILD_C
#include "build.h"

int main(int argc, char *argv[])
{
    INFO("This is info.");
	CMD("echo", "\"So, this is too.\"");

    return 0;
}
```

Now, just compile it.

```
cc build.c -I. -o build
```
