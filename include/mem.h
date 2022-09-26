#ifndef rascal_mem_h
#define rascal_mem_h

#include "obj.h"
#include "utils/num.h"

// C types
typedef int     (*init_fn_t)(void *spc, val_t val);
typedef void    (*finalize_fn_t)(obj_t **buf);
typedef void    (*mark_fn_t)(obj_t *obj);
typedef obj_t  *(*resize_fn_t)(obj_t *obj, arity_t n);

typedef enum
  {
    mem_fl_sharing  =0x8000,
    mem_fl_protected=0x4000,
    mem_fl_static   =0x2000,
  } mem_fl_t;

  typedef struct heap_t
{
  obj_t   *objects;
  objs_t  *gray_stack;

  size_t allocated;
  size_t next_gc;

  bool_t collecting;
  float  heap_grow_factor;
} heap_t;

typedef struct
{
  size_t        base_obj_size;
  mark_fn_t     do_mark;
  finalize_fn_t do_finalize;
} obj_api_t;

// globals --------------------------------------------------------------------
extern obj_api_t ObjApis[num_val_types];
extern heap_t    Heap;

// forward declarations & generics --------------------------------------------
void  *alloc(size_t n);
void   dealloc(void *ptr, size_t n);
void  *resize( void *spc, size_t old, size_t new );
void   copy( void *dst, void *src, size_t n );
void  *duplicate( void *ptr, size_t n );

void   finalize( obj_t **buffer );

#define mark(val)				\
  _Generic((val),				\
	   val_t:mark_val,			\
	   obj_t*:mark_obj)(val)

void mark_val(val_t  val);
void mark_obj(obj_t *obj);

#define trace(val, ...)						\
  _Generic((val),						\
	   obj_t*:trace_obj,					\
	   val_t:trace_val,					\
	   obj_t**:trace_objs,					\
	   val_t*:trace_vals,					\
	   default:trace_noop)((val) __VA_OPT__(,) __VA_ARGS__)

void trace_obj(obj_t *obj);
void trace_val(val_t val);
void trace_objs(obj_t **objs, arity_t n);
void trace_vals(val_t *vals, arity_t n);
void trace_noop(void *ptr, arity_t n);

// toplevel dispatch ----------------------------------------------------------
void collect_garbage( void );

// initialization
void mem_init( void );

// convenience ----------------------------------------------------------------
#define scrub(spc, n)              memset((spc), 0, (n))

#define scrubv(vec, n, type)       scrub((vec), (n) * sizeof(type))
#define allocv(n, type)            alloc((n) * sizeof(type))
#define deallocv(ptr, n, type)     dealloc((ptr), (n) * sizeof(type))
#define duplicatev(ptr, n, type)   duplicate((ptr), (n) * sizeof(type))
#define resizev(ptr, on, nn, type) resize((ptr), (on) * sizeof(type), (nn) * sizeof(type))
#define copyv(dst, src, n, type)   copy((dst), (src), (n) * sizeof(type))

#define safe_alloc(func, ...)						\
  ({									\
    void *_ptr_ = func( __VA_ARGS__ );					\
    if ( _ptr_ == NULL )						\
      {									\
	printf("allocation failed at %s:%d:%s.\n",			\
	       __FILE__,						\
	       __LINE__,						\
	       __func__);						\
	exit(1);							\
      }									\
    _ptr_;								\
  })
  

#endif
