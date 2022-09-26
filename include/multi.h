#ifndef rascal_multi_h
#define rascal_multi_h

#include "obj/func.h"

/*	   
 */

typedef struct methods_t methods_t;
typedef struct cons_t    cons_t;

typedef struct bitmap_t
{
  arity_t   len;
  arity_t   cap;
  uint64_t *map;
} bitmap_t;

struct methods_t
{
  arity_t          position;       // position to dispatch on

  obj_t           *parent;         // parent method in case backtracking is needed
  objs_t           methods;
};

#endif
