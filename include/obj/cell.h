#ifndef rascal_cell_h
#define rascal_cell_h

#include "rl/value.h"
#include "obj/type.h"

typedef struct cons_t  cons_t;
typedef struct icons_t icons_t;
typedef struct pair_t  pair_t;

struct cons_t
{
  OBJECT
  size_t  length;
  value_t head;
  cons_t *tail;
};

struct icons_t
{
  OBJECT
  size_t  length;
  value_t head;
  value_t tail;
};

struct pair_t
{
  OBJECT
  hash_t  hash;
  value_t car;
  value_t cdr;
};

// globals
extern type_t ConsType, IConsType, PairType;

#endif
