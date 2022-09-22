#ifndef rascal_heap_h
#define rascal_heap_h

#include "obj.h"

typedef struct heap_t heap_t;

DECL_OBJ(heap);

struct heap_t
{
  OBJ_HEAD;

  obj_t *objects;
  obj_t *gray_stack;

  size_t allocated;
  size_t next_gc;

  bool_t collecting;
  float  heap_grow_factor;
};

// globals --------------------------------------------------------------------
extern heap_t *Heap;

#endif
