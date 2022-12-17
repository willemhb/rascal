#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "rascal.h"

#include "vm/obj/support/namespace.h"
#include "vm/obj/support/var.h"

/* commentary 

   Internal type representation (within the language types are represented by
   their constructors). 

   notes:
   
   - internal type representations are hash-consed for fast  */

/* C types */
typedef struct type_t type_t;
typedef struct alias_type_t alias_type_t;
typedef struct data_type_t data_type_t;
typedef struct primitive_type_t primitive_type_t;
typedef struct record_type_t record_type_t;
typedef struct union_type_t union_type_t;
typedef enum kind_t kind_t;

enum kind_t { none_kind,
	      primitive_alias_kind,
	      primitive_kind,
	      record_alias_kind,
	      record_kind,
	      union_alias_kind,
	      union_kind,
	      any_kind
};

struct type_t {
  ulong hash, idno;
  kind_t kind;
  bool builtin; // if true don't free on cleanup
};

struct alias_type_t {
  type_t type;
  rl_function_t *constructor;
  type_t *aliased;
  type_t *root; /* if aliased is also an alias this points to the underlying data type */
};

struct data_type_t {
  type_t type;
  value_type_t value_type;
  object_type_t object_type;
  int flags; /* default flags */
  int layout; /* default layout */
  size_t value_size;
  size_t object_size;
};

struct primitive_type_t {
  data_type_t data_type;
  rl_function_t *constructor;
};

struct record_type_t {
  data_type_t data_type;
  namespace_t *slots;
};

struct union_type_t {
  type_t type;

  type_t *left;
  type_t *right;
};

/* API */
type_t *make_record_type( var_t *binding, size_t n, rl_symbol_t **slot_names );
type_t *make_union_type( var_t *binding, size_t n, type_t **members );
type_t *make_alias_type( var_t *binding, type_t *aliased );

type_t *runtime_value_type( rl_value_t value );
type_t *runtime_object_type( rl_object_t *object );
type_t *proper_value_type( rl_value_t value );
type_t *proper_object_type( rl_object_t *object );

#define runtime_type( x )				\
  _Generic((x),						\
	   rl_value_t:runtime_value_type,		\
	   rl_object_t*:runtime_object_type)((x))

#define proper_type( x )				\
  _Generic((x),						\
	   rl_value_t:proper_value_type,		\
	   rl_object_t*:proper_object_type)((x))

/* runtime dispatch */
void rl_vm_type_init( void );
void rl_vm_type_mark( void );
void rl_vm_type_cleanup( void );

/* convenience */

#endif
