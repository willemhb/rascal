#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* value types */
typedef uword rl_value_t; // tagged value (unitype)
typedef bool rl_bool_t;
typedef ascii_t rl_glyph_t;
typedef FILE *rl_stream_t;
typedef struct rl_object_t rl_object_t;
typedef double rl_real_t;

/* user object types */
typedef struct rl_symbol_t rl_symbol_t;
typedef struct rl_function_t rl_function_t; /* A rascal generic function */
typedef struct rl_cons_t rl_cons_t;
typedef struct rl_string_t rl_string_t;
typedef struct rl_vector_t rl_vector_t;
typedef struct rl_dict_t rl_dict_t;
typedef struct rl_set_t rl_set_t;
typedef struct rl_record_t rl_record_t;

/* internal object types */
typedef struct rl_module_t rl_module_t; /* a unit of compiled code, plus basic metadata */
typedef struct rl_control_t rl_control_t;
typedef struct rl_namespace_t rl_namespace_t;
typedef struct rl_variable_t rl_variable_t;

/* internal types (not first class values) */
typedef struct alist_t stack_t;

typedef struct vm_t vm_t;
typedef struct reader_t reader_t;
typedef struct heap_t heap_t;

/* type code types */
typedef enum value_type_t value_type_t;
typedef enum object_type_t object_type_t;

enum value_type_t {
  bool_value, glyph_value, stream_value, object_value, real_value
};

enum object_type_t {
  nul_object=real_value+1, symbol_object, function_object, cons_object,
  string_object, vector_object, dict_object, set_object, record_object,

  module_object, control_object, namespace_object, variable_object
};

/* internal function pointer types */
typedef size_t (*pad_array_size_fn_t)(size_t new_count);

#endif
