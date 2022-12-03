#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* tagged value type */
typedef uword value_t;

/* immediate types */
typedef double    real_t;
typedef uint64_t  fixnum_t; // 48-bit unsigned integer (defines size limit)
typedef bool      bool_t;
typedef ascii_t   glyph_t;
typedef FILE     *stream_t;
typedef value_t (*native_t)(value_t *args, int nargs);

/* object types */
typedef struct object_t    object_t;
typedef struct symbol_t    symbol_t;
typedef struct cons_t      cons_t;
typedef struct rl_string_t rl_string_t;
typedef struct type_t      type_t;
typedef struct datatype_t  datatype_t;
typedef struct uniontype_t uniontype_t;
typedef struct control_t   control_t;
typedef struct lambda_t    lambda_t;
typedef struct closure_t   closure_t;

/* internal structure/array types types */
typedef struct alist_t  stack_t;

typedef ushort  *bytecode_t;
typedef value_t *vector_t;
typedef char    *string_t;

typedef struct namespc_t namespc_t;
typedef struct envt_t    envt_t;

typedef struct vm_t  vm_t;
typedef struct reader_t reader_t;
typedef struct heap_t heap_t;

/* internal function pointer types */
typedef int    (*validate_syntax_fn_t)(cons_t *form, int fl);
typedef int    (*reader_dispatch_fn_t)(reader_t *reader, int dispatch);
typedef size_t (*pad_array_size_fn_t)(size_t new_len, size_t old_len, size_t old_cap);
typedef void   (*object_runtime_fn_t)(object_t *object);
typedef bool   (*isa_fn_t)(type_t *self, value_t x);

#endif
