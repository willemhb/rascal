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
typedef int32_t               small_t;
typedef ascii_t               glyph_t;
typedef value_t             (*native_fn_t)(value_t *args, size_t nargs);

/*object types */
/* generic object type (common header) */
typedef struct object_t       object_t;

/* user object types */
typedef struct symbol_t       symbol_t;
typedef struct cons_t         cons_t;
typedef struct vector_t       vector_t;
typedef struct string_t       string_t;
typedef struct bytes_t        bytes_t;
typedef struct map_t          map_t;
typedef struct set_t          set_t;
typedef struct bigint_t       bigint_t;
typedef struct ratio_t        ratio_t;
typedef struct record_t       record_t;

/* IO object types */
typedef struct stream_t        stream_t;
typedef struct text_buffer_t   text_buffer_t;
typedef struct binary_buffer_t binary_buffer_t;
typedef struct read_table_t    read_table_t;
typedef struct reader_t        reader_t;

/* function object types */
typedef struct function_t     function_t;
typedef struct method_t       method_t;
typedef struct native_t       native_t;
typedef struct lambda_t       lambda_t;
typedef struct script_t       script_t;
typedef struct control_t      control_t;

/* vm object types */
typedef struct type_t         type_t;
typedef struct bytecode_t     bytecode_t;
typedef struct closure_t      closure_t;
typedef struct namespace_t    namespace_t;
typedef struct environment_t  environment_t;

/* internal structure types (no rascal representation) */
typedef struct vm_t   vm_t;
typedef struct heap_t heap_t;

#endif
