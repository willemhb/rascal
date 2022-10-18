#ifndef rascal_obj_memory_h
#define rascal_obj_memory_h

#include "obj/array.h"


// heap type
struct heap_t
{
  object_t    *objects;
  stack_t     *grays;

  size_t      allocated;
  size_t      alloccap;

  bool        readBarrier;
  bool        writeBarrier;
};


#endif
