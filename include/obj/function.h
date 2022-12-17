#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "vm/object.h"

/* commentary

   a rascal function.

   as in julia, all rascal functions are generic.

   functions are also the common way of referring to types. */

/* C types */
typedef enum fn_flags_t fn_flags_t;

enum fn_flags_t {
    is_primitive_method,
    is_native_method,
    is_compiled_method,
    is_constructor=4,
    is_macro=8,
    is_vargs=16,
  };

struct rl_function_t {
  RL_OBJ_HEADER;

  rl_function_t *next; // method table is an invasive linked list
  rl_symbol_t *name;
  type_t *type;
  signature_t *signature;

  union {
    
  };
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
