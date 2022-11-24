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
typedef int       (*compare_fn_t)(value_t x, value_t y);
typedef ulong     (*hash_fn_t)(value_t x);

/* union of types a tagged value can be */
typedef union
{
  value_t      as_value;
  real_t       as_real;
  bool_t       as_bool;
  fixnum_t     as_fixnum;
  smint_t      as_smint;
  glyph_t      as_glyph;

  void        *as_pointer;
  stream_t    *as_stream;
  native_fn_t  as_native_fn;
  object_t    *as_object;
} rl_data_t;

#define QNAN    0x7ff8000000000000ul

#define TMASK   0xffff000000000000ul
#define PMASK   (~TMASK)

#define NUL     0x7ffc000000000000ul
#define TRUE    0x7ffd000000000001ul
#define FALSE   0x7ffd000000000000ul
#define GLYPH   0x7ffe000000000000ul
#define FIXNUM  0x7fff000000000000ul
#define SMINT   0xfffc000000000000ul
#define NATIVE  0xfffd000000000000ul
#define STREAM  0xfffe000000000000ul
#define OBJECT  0xffff000000000000ul

/* convenience & utilities */
#define as_value( x )   (((rl_data_t)(x)).as_value)

#define as_real( x )    (((rl_data_t)(x)).as_real)
#define as_fixnum( x )  ((fixnum_t)untag(x))
#define as_smint( x )   ((smint_t)untag(x))
#define as_bool( x )    ((bool_t)untag(x))
#define as_glyph( x )   ((glyph_t)untag(x))

#define as_pointer( x ) ((void*)untag(x))
#define as_stream( x )  ((stream_t*)as_pointer(x))
#define as_object( x )  ((object_t*)as_pointer(x))
#define as_native( x )  ((native_fn_t)as_pointer(x))

#define untag( x )     (as_value(x)&PMASK)
#define tag( x, t )    (untag(x)|(t))

#endif
