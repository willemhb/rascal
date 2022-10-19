#ifndef rascal_obj_memory_h
#define rascal_obj_memory_h

#include "obj/array.h"

typedef struct heap_t    heap_t;
typedef struct objects_t objects_t;

// heap type
struct heap_t
{
  OBJECT

  heap_object_t *live_objects;
  object_t      *grays;

  size_t      allocated;
  size_t      alloccap;

  bool        readBarrier;
  bool        writeBarrier;
};

struct objects_t
{
  DARRAY(object_t*)
};

// globals
extern type_t *HeapType, *ObjectsType;

// forward declarations
void push_gray( heap_t *heap, object_t *obj );
void memory_init( void );

#endif
