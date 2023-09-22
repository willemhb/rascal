#ifndef rascal_util_io_h
#define rascal_util_io_h

#include <stdio.h>

#include "common.h"

int   fpeekc(FILE* ios);
char* readFile(const char* path);
void  runFile(const char* path);

#endif
