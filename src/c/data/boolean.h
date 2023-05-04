#ifndef data_boolean_h
#define data_boolean_h

#include "data/value.h"

// APIs & utilities
#define is_boolean(x) rl_isa(x, BOOLEAN)
#define as_boolean(x) rl_asa(x, IVMASK, boolean_t)

// globals
#define BOOLEANTAG (IMMTAG | (((uword)BOOLEAN) << 32))

#define TRUE       (BOOLEANTAG | 1)
#define FALSE      (BOOLEANTAG | 0)

#endif
