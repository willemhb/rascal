#ifndef rl_obj_module_h
#define rl_obj_module_h

#include "rascal.h"

#include "vm/obj.h"

#include "vm/obj/support/vector.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/vals.h"
#include "vm/obj/support/objs.h"

/* commentary 

   internal types used during compilation and to run compiled code. */

/* C types */
typedef enum var_fl_t var_fl_t;
typedef enum ns_fl_t ns_fl_t;
typedef enum cls_fl_t cls_fl_t;

enum var_fl_t {
  closure_var, /* stored in envt, fast access */
  upval_var, /* captured local, stored in envt unless nested scope outlives calling scope */
  package_var, /* global scope, namespaced to a module (can be imported from elsewhere) */
  toplevel_var, /* global scope, not namespaced (visible everywhere) */

  is_macro=4,
  is_captured=8,
  is_bound=16,
  is_exported=32,
};

enum cls_fl_t {
  is_pkg    =1,
  is_loaded =2,
};

struct ns_t {
  OBJ;

  
};

typedef struct bind_t {
  OBJ;
  size_t  index; // its location within the relevant environment
  val_t   value; // its value (if its value is stored directly)
} bind_t;

struct var_t {
  bind_t  bind;
  sym_t  *name;   // the plain name of the variable
  ns_t   *ns;     // the namespace for which the variable was originally defined
  type_t *type;   // applicable type constraint (if any)
};

struct upval_t {
  bind_t   bind;
  upval_t *next;
};

struct module_t {
  OBJ;

  sym_t      *name;
  ns_t       *ns;
  bytecode_t  bytecode;
  vector_t    constants;

  package_t  *package; /* the package level scope in which the current module is compiling (NULL if global). */
  module_t   *parent;  /* the parent of this module (either the package or the nesting scope). */
};

struct closure_t {
  OBJ;

  module_t  *module;
  objs_t    *bindings; /* upvalues (case of proper closure) or var objects (case of package) */
};

/* API */

/* runtime dispatch */
void rl_obj_module_init( void );
void rl_obj_module_mark( void );
void rl_obj_module_cleanup( void );

/* convenience */

#endif
