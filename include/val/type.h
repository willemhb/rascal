#ifndef rl_type_h
#define rl_type_h

#include "val/object.h"

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

/* ID numbers for builtin types */
enum {
  NONE_TYPE    =0x01,
  NUL_TYPE     =0x02,
  BOOL_TYPE    =0x03,
  CPTR_TYPE    =0x04,
  FPTR_TYPE    =0x05,
  TYPE_TYPE    =0x06,
  CLOSURE_TYPE =0x07,
  NATIVE_TYPE  =0x08,
  GENERIC_TYPE =0x09,
  MTROOT_TYPE  =0x0a,
  MTNODE_TYPE  =0x0b,
  MTLEAF_TYPE  =0x0c,
  CONTROL_TYPE =0x0d,
  SYMBOL_TYPE  =0x0e,
  ENVIRON_TYPE =0x0f,
  BINDING_TYPE =0x10,
  UPVALUE_TYPE =0x11,
  GLYPH_TYPE   =0x12,
  PORT_TYPE    =0x13,
  STRING_TYPE  =0x14,
  BINARY_TYPE  =0x15,
  MUTSTR_TYPE  =0x16,
  MUTBIN_TYPE  =0x17,
  ARITY_TYPE   =0x18,
  SMALL_TYPE   =0x19,
  REAL_TYPE    =0x1a,
  BIG_TYPE     =0x1b,
  RATIO_TYPE   =0x1c,
  PAIR_TYPE    =0x1d,
  LIST_TYPE    =0x1e,
  MUTPAIR_TYPE =0x1f,
  MUTLIST_TYPE =0x20,
  VECTOR_TYPE  =0x21,
  VECNODE_TYPE =0x22,
  MUTVEC_TYPE  =0x23,
  ALIST_TYPE   =0x24,
  MAP_TYPE     =0x25,
  MAPNODE_TYPE =0x26,
  MUTMAP_TYPE  =0x27,
  STRCACHE_TYPE=0x28,
  ENVMAP_TYPE  =0x29,
  ANY_TYPE     =0x2a
};

// user metaobject types
struct Type {
  HEADER;

  // bit fields
  word_t kind    : 3;
  word_t builtin : 1;

  // data fields
  word_t idno;         // unique identifier (basis of hash)

  // layout information
  Value  value_type;   // tag for values of this type
  size_t value_size;   // machine size of the data part of the tagged word
  size_t object_size;  // base size of objects of this type

  // type spec fields
  Symbol*    name;     // readable name under which the type was defined (if any)
  Environ*   slots;    // record or struct slots
  Set*       members;  // union members

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
Type* get_union_type(size_t n, Type* ts);

#endif
