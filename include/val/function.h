#ifndef rl_val_function_h
#define rl_val_function_h

#include "runtime.h"

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */

#define FN_HEADER                               \
  HEADER;                                       \
  Sym*   name;                                  \
  size32 arity;                                 \
  bool   variadic

struct Fn {
  FN_HEADER;
};

struct PrimFn {
  FN_HEADER;

  Opcode    label;   // if non-zero, this function's action is an inlineable VM label
  BuiltinFn builtin; // if non-null, call this function with the vm and arity as arguments
};

struct UserFn {
  FN_HEADER;

  Ns*     ns;   // namespace in which the function was compiled
  Env*    env;  // local variables
  Buffer* code; // instruction sequence
  Alist*  vals; // compiler constants
  Alist*  cls;  // closure (array of upvalues; NULL if this is a function prototype)
};

/* External APIs */
// general function APIs
#define as_fn(x)   ((Fn*)as_obj(x))
#define fn_name(x) ((x)->name->n->cs)
#define fn_ac(x)   ((x)->arity)
#define fn_va(x)   ((x)->variadic)

// primitive function APIs
#define is_primfn(x) has_type(x, T_PRIMFN)
#define as_primfn(x) ((PrimFn*)as_obj(x))

// user function APIs
#define is_userfn(x) has_type(x, T_USERFN)
#define as_userfn(x) ((UserFn*)as_obj(x))

#endif
