#ifndef rl_obj_module_h
#define rl_obj_module_h

#include "rascal.h"

#include "vm/obj.h"

#include "vm/obj/support/vector.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/vals.h"

/* commentary 

   internal types used during compilation and to run compiled code. */

/* C types */
typedef enum var_fl_t var_fl_t;
typedef enum ns_fl_t ns_fl_t;

enum var_fl_t {
  local_var, /* stored on stack, fast access */
  module_var, /* module scope (toplevel of file) */
  toplevel_var, /* toplevel scope (repl or import) */
  upval_var, /* */

  is_macro=4,
  is_captured=8,
  is_bound=16,
  is_exported=32,
};

struct ns_t {
  OBJ;

  
};

struct var_t {
  OBJ;

  sym_t *name;

  union {
    ns_t *ns; // special case of toplevel variable
    module_t *module; // all other variables
  };

  union {
    func_t *macro; // special case of macros
    size_t  offset;
    val_t   bind;
  };
};

struct upval_t {
  OBJ;

  
};

struct module_t {
  OBJ;

  module_t   *parent;
  sym_t      *name;
  str_t      *path;
  ns_t       *ns;
  vals_t     *binds;
  bytecode_t  bytecode;
  vector_t    constants;
};


/* API */

/* runtime dispatch */
void rl_obj_module_init( void );
void rl_obj_module_mark( void );
void rl_obj_module_cleanup( void );

/* convenience */

#endif
