#ifndef rascal_types_h
#define rascal_types_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here
// representation types

// tags
#define QNAN        0x7ffc000000000000ul
#define SIGN        0x8000000000000000ul

#define TAG_ATOM    0x7ffc000000000000ul
#define TAG_PNTR    0x7ffd000000000000ul
#define TAG_LIST    0x7ffe000000000000ul
#define TAG_FUNC    0x7fff000000000000ul
#define TAG_VEC     0xfffc000000000000ul
#define TAG_DICT    0xfffd000000000000ul
#define TAG_SET     0xfffe000000000000ul
#define TAG_CVALUE  0xffff000000000000ul

#define TAG_SYMBOL (ATOMIC|0ul)
#define TAG_CHAR   (ATOMIC|1ul)
#define TAG_BOOL   (ATOMIC|2ul)
#define TAG_INT    (ATOMIC|3ul)
#define TAG_PRIM   (ATOMIC|4ul)
#define TAG_TYPE   (ATOMIC|5ul)
#define TAG_CTYPE  (ATOMIC|6ul)

#define EOS       ((value_t)EOF|TAG_CHAR)
#define NUL       ((value_t)  0|TAG_LIST)

#define PMASK     0x0000fffffffffff0ul
#define TMASK     0xffff000000000000ul

// typedefs
typedef uintptr_t value_t;
typedef double    real_t;
typedef char      char_t;
typedef bool      bool_t;
typedef int64_t   int_t;
typedef void     *ptr_t;
typedef uint32_t  type_t;
typedef uint32_t  Ctype_t;
typedef uint16_t  opcode_t;

typedef struct object_t    object_t;
typedef struct symbol_t    symbol_t;
typedef struct cons_t      cons_t;
typedef struct vector_t    vector_t;
typedef struct dict_t      dict_t;
typedef struct set_t       set_t;
typedef struct function_t  function_t;
typedef struct cvalue_t    cvalue_t;
typedef struct stream_t    stream_t;

// C function typedefs
typedef value_t (*native_fn_t)(value_t *args, arity_t n);
typedef void    (*reader_fn_t)(stream_t *stream, char32_t dispatch);
typedef void    (*printer_fn_t)(stream_t *stream, value_t value);
typedef void    (*mark_fn_t)(object_t *obj);
typedef void    (*free_fn_t)(object_t *obj);
typedef value_t (*native_fn_t)(value_t *args, arity_t n);


// union types
typedef union
{
  real_t  as_real;
  value_t   as_val;
  void   *as_ptr;
  object_t  *as_obj;
} val_data_t;

// forward declarations
type_t typeof_val(value_t value);
type_t typeof_obj(object_t *obj);

#define rtypeof(val)				\
  _Generic((val),				\
	   value_t:typeof_val,			\
	   object_t*:typeof_obj)((val))

#endif
