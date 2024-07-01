#ifndef rl_util_io_h
#define rl_util_io_h

#include <stdio.h>

#include "error.h"

/* Various IO utilities (mostly safe versions of <stdio.h> utilities). */
Error rl_open(const char* path, const char* mode, FILE** buffer);
bool  rl_file_exists(const char* path);

#endif
