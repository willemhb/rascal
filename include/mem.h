#ifndef rascal_mem_h
#define rascal_mem_h

#include "obj/obj.h"
#include "utils/num.h"

// C types --------------------------------------------------------------------
typedef struct init_t
{
  type_t   type;
  flags_t  fl;
  arity_t  n;
  void    *data;
} init_t;

enum
  {
   SHARE_DATA   = 0x00001000,
   FAMOUS_OBJ   = 0x00002000,
   STATIC_OBJ   = 0x00004000,
   STATIC_DATA  = 0x00008000,
   INIT_NONE    = 0x00010000,
   INIT_SPECIAL = 0x00020000,
   INIT_PARENT  = 0x00040000,
   INIT_STACK   = 0x00080000,
   INIT_SEQ     = 0x00100000,
  };

// forward declarations & generics --------------------------------------------
void  *alloc(size_t n);
void   dealloc(void *ptr, size_t n);
void   copy( void *dst, void *src, size_t n );

obj_t *new( init_t *ini );
void   init(obj_t *obj, init_t *ini );
void   finalize( obj_t **buffer );

#define resize(ptr, ...)						\
  _Generic((ptr),							\
	   obj_t*: resize_obj,						\
	   default:resize_bytes)((ptr) __VA_OPT__(,) __VA_ARGS__)

obj_t *resize_obj( obj_t *obj, size_t old_n, size_t new_n );
void  *resize_bytes(void *ptr, size_t old_n, size_t new_n);

#define dup(ptr, ...)						\
  _Generic((ptr),						\
	   void*:dup_bytes,					\
	   obj_t*:dup_obj)((ptr) __VA_OPT__(,) __VA_ARGS__)

void  *dup_bytes(void *ptr, size_t n);
obj_t *dup_obj( obj_t *ptr );

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

// convenience ----------------------------------------------------------------
#define scrub(spc, n)              memset((spc), 0, (n))

#define scrubv(vec, n, type)       scrub((vec), (n) * sizeof(type))
#define allocv(n, type)            alloc((n) * sizeof(type))
#define deallocv(ptr, n, type)     dealloc((ptr), (n) * sizeof(type))
#define dupv(ptr, n, type)         dup((ptr), (n) * sizeof(type))
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
