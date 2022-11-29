#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* tagged value type */
typedef uword value_t;

/* immediate types */
typedef double real_t;
typedef ascii_t glyph_t;
typedef FILE *stream_t;

/* object types */
typedef struct object_t    object_t;
typedef struct type_t      type_t;
typedef struct datatype_t  datatype_t;
typedef struct uniontype_t uniontype_t;
typedef struct control_t   control_t;
typedef struct lambda_t    lambda_t;

/* internal structure/array types types */
typedef struct alist_t  stack_t;

typedef ushort  *bytecode_t;
typedef value_t *vector_t;
typedef char    *string_t;

typedef struct vm_t  vm_t;
typedef struct reader_t reader_t;
typedef struct heap_t heap_t;

#endif
