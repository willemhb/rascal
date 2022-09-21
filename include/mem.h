#ifndef rascal_mem_h
#define rascal_mem_h

#include "obj/obj.h"
#include "obj/num.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   STATIC_OBJ   =  1,
   STATIC_DATA  =  2,
   INIT_SPECIAL =  4,
   INIT_PARENT  =  8,
   INIT_STACK   = 16
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
extern heap_t     *TheHeap;

// forward declarations & generics --------------------------------------------
void  *alloc(size_t n);
void  *resize(void *ptr, size_t old, size_t new);
void   dealloc(void *ptr, size_t n);

obj_t *construct(val_type_t type, arity_t n, void *ini, flags_t fl);
void   initialize(obj_t *obj,val_type_t type,arity_t n,void *ini,flags_t fl);
void   finalize(obj_t *obj);

#define copy(ptr, ...)					\
  _Generic((ptr),					\
	   void*:copy_bytes,				\
	   obj_t*:copy_obj,				\
	   )((ptr) __VA_OPT__(,) __VA_ARGS__)

obj_t *copy_obj( obj_t *dst, obj_t *src );
void  *copy_bytes(void *dst, void *src, size_t n);

#define dup(ptr, ...)				\
  _Generic((ptr),				\
	   void*:dup_bytes,			\
	   obj_t*:dup_obj)

void  *dup_bytes(void *ptr, size_t n);
obj_t *dup_obj( obj_t *ptr );

#define trace(val, ...)						\
  _Generic((val),						\
	   obj_t*:trace_obj,					\
	   val_t:trace_val,					\
	   obj_t**:trace_objs,					\
	   val_t*:trace_vals,					\
	   default:trace_noop)((val) __VA_OPT__(,) __VA_ARGS__)

void   trace_obj(obj_t *obj);
void   trace_val(val_t val);
void   trace_objs(obj_t **objs, arity_t n);
void   trace_vals(val_t *vals, arity_t n);
void   trace_noop(void *ptr, arity_t n);

// toplevel dispatch ----------------------------------------------------------
void collect_garbage( void );
void mem_mark( void );
void mem_init( void );

// convenience ----------------------------------------------------------------
#define scrubv(vec, n, type)     memset((vec), 0, (n) * sizeof(type))
#define allocv(n, type)          alloc((n) * sizeof(type))
#define deallocv(ptr, n, type)   dealloc((ptr), (n) * sizeof(type))
#define dupv(ptr, n, type)       dup((ptr), (n) * sizeof(type))
#define resizev(ptr, n, type)    resize((ptr), (n) * sizeof(type))
#define copyv(dst, src, n, type) copy((dst), (src), (n) * sizeof(type))

#endif
