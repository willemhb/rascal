#ifndef util_io_h
#define util_io_h

#include <stdio.h>

#include "common.h"

/* API */
// misc io --------------------------------------------------------------------
void newln(void);
void fnewln(FILE* ios);
int peekc(void);
int fpeekc(FILE* ios);
void cleanf(FILE** ios);

#define SAFE_OPEN(func, args...)					\
  ({									\
    FILE* __file = func(args);						\
    if (__file == NULL) {						\
      fprintf(stderr, #func" failed. Reason: %s.\n", strerror(errno));	\
      exit(1);								\
    }									\
    __file;								\
  })

#endif
