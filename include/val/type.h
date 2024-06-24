#ifndef rl_val_type_h
#define rl_val_type_h

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
  CNTL_TYPE    =0x0d,
  SYM_TYPE  =0x0e,
  ENV_TYPE =0x0f,
  REF_TYPE =0x10,
  UPVAL_TYPE =0x11,
  GLYPH_TYPE   =0x12,
  PORT_TYPE    =0x13,
  STR_TYPE  =0x14,
  BIN_TYPE  =0x15,
  MSTR_TYPE  =0x16,
  MBIN_TYPE  =0x17,
  ARITY_TYPE   =0x18,
  SMALL_TYPE   =0x19,
  REAL_TYPE    =0x1a,
  BIG_TYPE     =0x1b,
  RATIO_TYPE   =0x1c,
  PAIR_TYPE    =0x1d,
  LIST_TYPE    =0x1e,
  MPAIR_TYPE =0x1f,
  MLIST_TYPE =0x20,
  VEC_TYPE  =0x21,
  VNODE_TYPE =0x22,
  MVEC_TYPE  =0x23,
  ALIST_TYPE   =0x24,
  MAP_TYPE     =0x25,
  MNODE_TYPE =0x26,
  MMAP_TYPE  =0x27,
  SCACHE_TYPE=0x28,
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
  Val    val_type;   // tag for values of this type
  size_t val_size;   // machine size of the data part of the tagged word
  size_t obj_size;  // base size of objects of this type

  // type spec fields
  Sym* name;     // readable name under which the type was defined (if any)
  Env* slots;    // record or struct slots
  Set* members;  // union members

  // constructor
  Obj* ctor;

  // lifetime & object methods
  rl_trace_fn_t  trace_fn;     // called to trace an object's owned pointers
  rl_clone_fn_t  clone_fn;     // called to duplicate an object's owned pointers
  rl_free_fn_t   free_fn;      // called to free an object's owned pointers
  rl_sizeof_fn_t sizeof_fn;    // called to determine an object's overallocated size

  // comparison methods
  rl_hash_fn_t  hash_fn;
  rl_egal_fn_t  egal_fn;
  rl_order_fn_t ord_fn;
};

/* Globals */
extern Type TypeType, NoneType, AnyType;

/* APIs */
/* Type APIs */
#define is_type(x) has_type(x, &TypeType)
#define as_type(x) ((Type*)as_obj(x))

bool  has_instance(Type* p, Type* t);
char* t_name(Type* x);
Type* get_union_type(size_t n, Type* ts);
void  init_builtin_type(Type* t, char* name, rl_native_fn_t ctor);

#endif
