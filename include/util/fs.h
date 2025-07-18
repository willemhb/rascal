#ifndef rl_util_fs_h
#define rl_util_fs_h

/**
 *
 * Miscellaneous IO utilities.
 *
 **/

// headers --------------------------------------------------------------------
#include <stdio.h>

#include "common.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
size_t file_size(FILE* file);
char* read_file(char* fname);
char* read_stream(FILE* in);

// initialization -------------------------------------------------------------

#endif
