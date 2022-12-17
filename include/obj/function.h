#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "vm/object.h"

/* commentary

   a rascal function.

   as in julia, all rascal functions are generic.

   functions are also the common way of referring to types. */

/* C types */
typedef struct method_t method_t;
typedef struct method_signature_t method_signature_t;
typedef enum method_type_t method_type_t;
typedef struct method_cache_t method_cache_t;

enum method_type_t {
    invalid_method_type=-1,
    primitive_method_type,
    native_method_type,
    user_method_type,
  };

struct rl_function_t {
  RL_OBJ_HEADER;

  rl_string_t *name;
  type_t *type;
  method_t *methods;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
