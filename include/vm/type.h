#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "rascal.h"

#include "vm/obj.h"

/* commentary 

   Internal type representation (within the language types are represented by
   their constructors). 

   notes:

   - internal type representations are hash-consed for fast  */

/* C types */
typedef enum kind_t kind_t;
typedef struct dtype_t dtype_t;

enum kind_t {
  none_alias_kind,
  none_kind,
  primitive_alias_kind,
  primitive_kind,
  record_alias_kind,
  record_kind,
  union_alias_kind,
  union_kind,
  any_alias_kind,
  any_kind
};

struct type_t {
  OBJ;
  ulong hash, idno;
};

struct atype_t {
  type_t type;
  func_t *constructor;
  type_t *aliased;
  type_t *root; /* if aliased is also an alias this points to the underlying data type */
};

struct dtype_t {
  type_t type;
  val_type_t val_type;
  obj_type_t obj_type;
  size_t value_size;
  size_t object_size;
};

struct ptype_t {
  dtype_t data_type;
  func_t *constructor;
};

struct record_type_t {
  dtype_t data_type;
  ns_t *slots;
};

struct utype_t {
  type_t type;

  signature_t *members;
};

/* API */
type_t *make_record_type( var_t *binding, size_t n, sym_t **slot_names );
type_t *make_union_type( var_t *binding, size_t n, type_t **members );
type_t *make_alias_type( var_t *binding, type_t *aliased );

/* runtime dispatch */
void rl_vm_type_init( void );
void rl_vm_type_mark( void );
void rl_vm_type_cleanup( void );

/* convenience */

#endif
