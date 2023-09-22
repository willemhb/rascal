#ifndef rascal_util_io_h
#define rascal_util_io_h

#include <stdio.h>

#include "common.h"

#define SAFE_OPEN(path, ...)                                 \
  ({                                                         \
    FILE* __file = fopen(path __VA_OPT__(,) __VA_ARGS__);    \
    if (__file == NULL) {                                    \
      fprintf(stderr,                                        \
              "could not read file \"%s\" at %s:%s:%d.\n",   \
              path,                                          \
              __FILE__,                                      \
              __func__,                                      \
              __LINE__);                                     \
      exit(74);                                              \
    }                                                        \
    __file;                                                  \
  })

#define SAFE_READ(path, buffer, runeType, fileSize, file)               \
  do {                                                                  \
    size_t __runesRead = fread(buffer,                                  \
                               sizeof(runeType),                        \
                               fileSize,                                \
                               file);                                   \
    if (__runesRead < fileSize) {                                       \
      fprintf(stderr,                                                   \
              "Could not read file \"%s\".\n",                          \
              path);                                                    \
      exit(74);                                                         \
    }                                                                   \
    buffer[__runesRead] = (runeType)0;                                  \
  } while (false)

int    fpeekc(FILE* ios);
char*  readFile(const char* path);
size_t fileSize(FILE* ios);

#endif
