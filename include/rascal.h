#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* tagged value type */
typedef uword rl_value_t;

/* immediate types */
typedef double       rl_real_t;
typedef uint64_t     rl_fixnum_t; // 48-bit unsigned integer (defines size limit)
typedef bool         rl_bool_t;
typedef ascii_t      rl_glyph_t;
typedef int          rl_small_t;
typedef FILE        *rl_stream_t;
typedef nullptr_t    rl_nul_t;
typedef rl_value_t (*rl_native_t)(rl_value_t *args, size_t nargs);

/* user object types */
typedef struct rl_object_t rl_object_t;
typedef struct rl_symbol_t rl_symbol_t;
typedef struct rl_function_t rl_function_t;
typedef struct rl_cons_t rl_cons_t;
typedef struct rl_string_t rl_string_t;
typedef struct rl_vector_t rl_vector_t;
typedef struct rl_table_t rl_table_t;
typedef struct rl_u16_array_t rl_u16_array_t;
typedef struct rl_record_t rl_record_t;

/* metaobject types */
typedef struct rl_type_t rl_type_t;
typedef struct rl_alias_type_t rl_alias_type_t;
typedef struct rl_union_type_t rl_union_type_t;
typedef struct rl_data_type_t rl_data_type_t;
typedef struct rl_data_type_t rl_primitive_type_t;
typedef struct rl_record_type_t rl_record_type_t;

/* VM types */
typedef struct rl_control_t     rl_control_t;
typedef struct rl_bytecode_t    rl_bytecode_t;
typedef struct rl_closure_t     rl_closure_t;
typedef struct rl_namespace_t   rl_namespace_t;
typedef struct rl_variable_t    rl_variable_t;

/* internal structure/array types types */
typedef struct alist_t stack_t;

typedef ushort     *bytecode_t;
typedef rl_value_t *vector_t;
typedef ascii_t    *string_t;
typedef rl_type_t  *signature_t;

typedef struct vm_t     vm_t;
typedef struct reader_t reader_t;
typedef struct heap_t   heap_t;

/* internal function pointer types */
typedef int    (*validate_syntax_fn_t)(rl_cons_t *form, int fl);
typedef int    (*reader_dispatch_fn_t)(reader_t *reader, int dispatch);
typedef size_t (*pad_array_size_fn_t)(size_t new_len, size_t old_len, size_t old_cap);
typedef void   (*object_runtime_fn_t)(rl_object_t *object);
typedef bool   (*isa_fn_t)(rl_type_t *self, rl_value_t x);

#endif
