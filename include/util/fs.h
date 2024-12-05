#ifndef rl_util_fs_h
#define rl_util_fs_h

#include "common.h"

/* File system utilities. */
size_t home_dir(char** buffer, size_t buffer_size);
size_t abs_path(char* path, char** buffer, size_t buffer_size);

#endif
