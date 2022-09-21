#ifndef rascal_mem_h
#define rascal_mem_h

#include "obj/obj.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   MEM_STATIC  = 1,
   DATA_STATIC = 2,
  } mem_fl_t;

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
  OBJ_HEAD;

  obj_t *objects;
  obj_t *gray_stack;

  size_t allocated;
  size_t next_gc;

  bool_t collecting;
  float  heap_grow_factor;
} heap_t;

// globals --------------------------------------------------------------------
extern rt_table_t *RuntimeMethods[NUM_TYPES_PAD];
extern obj_t      *WellKnownObjects[NUM_TYPES_PAD];
extern heap_t     *TheHeap;

#define ObjList (TheHeap->object.next)

// forward declarations & generics --------------------------------------------
void  *allocate(size_t n);
void  *alloc_arr(size_t n, size_t ob_size);
void  *duplicate(void *ptr, size_t n);
obj_t *construct(val_type_t type, size_t n, void *ini, flags_t fl);
void   finalize(obj_t *obj);

#define trace(val, args...)			\
  _Generic((val),				\
	   obj_t*:trace_obj,			\
	   val_t:trace_val,			\
	   obj_t**:trace_objs,			\
	   val_t*:trace_vals,			\
	   default:trace_noop)((val) args)

void   trace_obj(obj_t *obj);
void   trace_val(val_t val);
void   trace_objs(obj_t **objs, arity_t n);
void   trace_vals(val_t *vals, arity_t n);
void   trace_noop(void *ptr, arity_t n);

#define reallocate(ptr, args...)			\
  _Generic((ptr),					\
	   obj_t*:reallocate_obj,			\
	   default:reallocate_bytes)((ptr) args)

void  *reallocate_bytes(void *ptr, size_t n);
obj_t *reallocate_obj(obj_t *obj, size_t n);

#endif
