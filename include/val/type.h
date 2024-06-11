#ifndef rl_type_h
#define rl_type_h

#include "object.h"

/* Types and APIs for the Rascal type system, as well as supporting globals. */
/* C types */

// encodes how membership in the type is determined
typedef enum Kind {
  // kinds ordered by heuristic specificity
  BOTTOM_TYPE,         // nothing - no values
  DATA_TYPE,           // a set of instantiable Rascal values
  UNION_TYPE,          // a union type with at least one abstract member
  TOP_TYPE             // union of all types
} Kind;

// user metaobject types
struct Type {
  HEADER;

  // bit fields
  word_t kind    : 3;
  word_t builtin : 1;

  // unique identifier (basis of hash value)
  word_t idno;

  // layout information
  Value      value_type;   // tag for values of this type
  size_t     value_size;
  size_t     object_size;

  // type spec fields
  Type*      parent;       // parent type (an abstract type, trait, or Any)
  Environ*   slots;        // 
  Set*       members;      // union or trait members

  // constructor
  Object* ctor;

  // lifetime & object methods
  rl_trace_fn_t    trace_fn;
  rl_destruct_fn_t destruct_fn;
  rl_sizeof_fn_t   sizeof_fn;

  // comparison methods
  rl_hash_fn_t  hash_fn;
  rl_egal_fn_t  egal_fn;
  rl_order_fn_t order_fn;
};

/* Globals */
extern Type TypeType, NoneType, AnyType;

/* APIs */
bool has_instance(Type* p, Type* t);

#endif
