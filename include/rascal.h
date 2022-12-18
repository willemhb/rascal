#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* value types */
typedef uword        val_t; // tagged value (unitype)
typedef bool         bool_t;
typedef ascii_t      glyph_t;
typedef ulong        fixnum_t;
typedef struct obj_t obj_t;
typedef double       real_t;

/* user object types */
typedef struct sym_t    sym_t;
typedef struct func_t   func_t; /* A rascal generic function */
typedef struct cons_t   cons_t;
typedef struct ios_t    ios_t;
typedef struct str_t    str_t;
typedef struct vec_t    vec_t;
typedef struct dict_t   dict_t;
typedef struct set_t    set_t;
typedef struct struct_t struct_t;

/* VM object types */
typedef struct native_t  native_t;  /* builtin C function */
typedef struct prim_t    prim_t;    /* builtin function corresponding to an opcode */
typedef struct module_t  module_t;  /* compiled code unit */
typedef struct closure_t closure_t; /* a module plus an execution context */
typedef struct methods_t methods_t; /* method table */
typedef struct cntl_t    cntl_t;    /* reified continuation */
typedef struct ns_t      ns_t;      /* namespace */
typedef struct var_t     var_t;     /* namespace entry */
typedef struct upval_t   upval_t;   /* closure entry */
typedef struct type_t    type_t;    /* type descriptor */

/* internal types (not first class values) */
typedef struct alist_t stack_t;

typedef struct vm_t vm_t;
typedef struct reader_t reader_t;
typedef struct heap_t heap_t;

/* type code types */
typedef enum val_type_t val_type_t;
typedef enum obj_type_t obj_type_t;

/* */
enum val_type_t {
  bool_val, glyph_val, fixnum_val, obj_val, real_val
};

enum obj_type_t {
  /* user object types */
  nul_obj=real_val+1, sym_obj, func_obj, cons_obj,
  ios_obj, str_obj, vec_obj, dict_obj, set_obj,
  struct_obj,

  /* internal object types exposed to the user */
  native_obj, prim_obj, module_obj, closure_obj,
  methods_obj, cntl_obj, ns_obj, var_obj, upval_obj,

  /* internal object types not exposed to the user */
  /* type objects */
  type_obj, atype_obj, ptype_obj, rtype_obj, utype_obj,

  /* hamt member types */
  vec_node_obj,
  dict_node_obj, dict_leaf_obj, dict_leaves_obj,
  set_node_obj, set_leaf_obj, set_leaves_obj,

  /* method table member types */
  method_obj,
};

/* internal function pointer types */
typedef size_t (*pad_array_size_fn_t)(size_t new_count);

#endif
