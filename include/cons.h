#ifndef rascal_cons_h
#define rascal_cons_h

#include <stddef.h>

#include "object.h"

// C types --------------------------------------------------------------------
typedef struct pair_t
{
  HEADER;
  value_t car, cdr;
} pair_t;

typedef struct entry_t
{
  pair_t base;
  ulong  hash;
} entry_t;

typedef struct pair_t cons_t;

// macros & statics -----------------------------------------------------------
#define aspair(x)  ((pair_t*)pval(x))
#define asentry(x) ((entry_t*)pval(x))

#define car(x)   (aspair(x)->car)
#define cdr(x)   (aspair(x)->cdr)
#define ehash(x) (asentry(x)->hash)

// forward declarations -------------------------------------------------------
bool ispair(value_t x);
bool iscons(value_t x);
bool isnil(value_t x);
bool islist(value_t x);

value_t pair(value_t ca, value_t cd);
value_t cons(value_t ca, value_t cd);
value_t cons_n(value_t *args, size_t n_args);

#endif
