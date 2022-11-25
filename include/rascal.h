#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types.
*/

/* C types */
/* tagged value type */
typedef uword                 value_t;

/* immediate types */
typedef void                 *nul_t;
typedef bool                  bool_t;
typedef double                real_t;
typedef uint64_t              fixnum_t;
typedef int32_t               smint_t;
typedef FILE                  stream_t;
typedef ascii_t               glyph_t;
typedef value_t             (*native_fn_t)(value_t *args, size_t nargs);

/*object types */
/* generic object type (common header) */
typedef struct object_t       object_t;

/* core user object types */
typedef struct cons_t         cons_t;
typedef struct atom_t         atom_t;
typedef struct vector_t       vector_t;
typedef struct string_t       string_t;
typedef struct map_t          map_t;

/* internal object types */
typedef struct type_t         type_t;
typedef struct bytecode_t     bytecode_t;
typedef struct lambda_t       lambda_t;
typedef struct control_t      control_t;
typedef struct closure_t      closure_t;
typedef struct environment_t  environment_t;
typedef struct namespace_t    namespace_t;

/* internal function pointer types */
typedef object_t *(*make_fn_t)(type_t *type, size_t n);
typedef void      (*init_fn_t)(object_t *object, size_t n, void *ini);
typedef void      (*trace_fn_t)(object_t *object);
typedef void      (*free_fn_t)(object_t *object);
typedef size_t    (*sizeof_fn_t)(object_t *object);
typedef int       (*compare_fn_t)(value_t x, value_t y, bool eq);
typedef int       (*compare_traverse_fn_t)(value_t x, value_t y, bool eq, );
typedef ulong     (*hash_fn_t)(value_t x);
typedef ulong     (*hash_traverse_fn_t)(value_t x, ulong accum, );

#endif
