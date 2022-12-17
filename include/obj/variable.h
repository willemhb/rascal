#ifndef rl_obj_variable_h
#define rl_obj_variable_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef enum var_type_t var_type_t;

enum var_type_t
  {
    var_type_unbound=-1,
    var_type_local,
    var_type_module,
    var_type_macro
  };

struct rl_variable_t
{
  RL_OBJ_HEADER;

  var_type_t var_type;
  bool exported;
  size_t offset; // location in corresponding environment where variable can be found

  rl_symbol_t *name;
  rl_namespace_t *namespace; // namespace in which the variable was originally bound
  rl_value_t _meta;
  rl_type_t *type;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_variable_init( void );
void rl_obj_variable_mark( void );
void rl_obj_variable_cleanup( void );

/* convenience */

#endif
