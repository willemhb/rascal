#ifndef rascal_memory_h
#define rascal_memory_h

#include "obj/obj.h"

// C types --------------------------------------------------------------------
typedef struct rt_table_t
{
  OBJ_HEAD;

  // basic parameters ---------------------------------------------------------
  obj_t     *name;
  size_t     base_size;
  val_t      val_tag;
  val_type_t val_type;
  
  // memory management --------------------------------------------------------
  obj_t *(*construct)(val_type_t type, arity_t n, void *ini, flags_t fl);
  void   (*initialize)(obj_t *obj, arity_t n, void *ini, flags_t fl);
  obj_t *(*reallocate)(obj_t *obj, size_t n);
  void   (*trace)(obj_t *obj);
  void   (*finalize)(obj_t *obj);
} rt_table_t;

typedef struct heap_t
{
  OBJ_HEAD; // We'll just use this as the object list!

  size_t allocated;
  size_t next_gc;

  bool_t collecting;
  float  heap_grow_factor;
} heap_t;

// globals --------------------------------------------------------------------
extern rt_table_t *RuntimeMethods[NUM_TYPES_PAD];
extern obj_t      *WellKnownObjects[NUM_TYPES_PAD];
extern heap_t     *TheHeap;

// forward declarations -------------------------------------------------------
void  *allocate(size_t n);
obj_t *construct(val_type_t type, size_t n, void *ini, flags_t fl);
obj_t *reallocate(obj_t *obj, size_t n);
void   trace(obj_t *obj);
void   finalize(obj_t *obj);

#endif
