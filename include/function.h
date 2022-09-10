#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"

// C types --------------------------------------------------------------------

typedef enum
  {
    /* initial part of the range reserved for builtin types */
    // immediate types --------------------------------------------------------
    type_nil         = 0,

    type_boolean     = 1,
    type_character   = 2,

    // number types -----------------------------------------------------------
    type_integer     = 3,
    type_flonum      = 4,
    type_ratio       = 5,
    type_complex     = 6,
    type_bigint      = 7,

    // pair types -------------------------------------------------------------
    type_pair        = 8,
    type_entry       = 9,
    type_cons        =10,

    // misc core types --------------------------------------------------------
    type_symbol      =11,
    type_function    =12,
    type_port        =13,

    // primitive array types --------------------------------------------------
    type_string      =14,
    type_bytes       =15,
    type_buffer      =16,

    // builtin collection types -----------------------------------------------
    type_vector      =17,
    type_tuple       =18,
    type_dict        =19,
    type_set         =20,
    type_table       =21,
    type_record      =22,

    // internal types ---------------------------------------------------------
    type_bytecode    =23,
    type_methods     =24,
    type_closure     =25,
    type_module      =26,
    type_stack       =27,
    type_heap        =28,
    type_continuation=29,
    type_symbol_table=30,
    type_name_space  =31,

    // builtin union types, top/bottom types ----------------------------------
    type_number      =32,
    type_list        =33,
    type_any         =34,
    type_none        =35,
    type_type        =36,
    num_builtin_types=37,

    // predicates -------------------------------------------------------------
  } builtin_t;

typedef struct
{
  HEADER;
  object_t *locals;
  object_t *nonlocals;
  object_t *namespace;
  object_t *constants;
  object_t *instructions;
} code_t;

typedef struct closure_t
{
  HEADER;

  object_t *template;
  object_t *environment;
} closure_t;

typedef struct closure_t module_t;

typedef struct
{
  HEADER;

  value_t   behavior;
  object_t *name;
  object_t *signature;

  bool      type;
  bool      macro;
  bool      vargs;
  builtin_t builtin;
} function_t;

#endif
