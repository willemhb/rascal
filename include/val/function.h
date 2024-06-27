#ifndef rl_function_h
#define rl_function_h

#include "opcodes.h"

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */
#define FUNC_HEADER                             \
  HEADER;                                       \
  /* bit fields */                              \
  word_t va : 1;                                \
  /* data fields */                             \
  uint nargs;                                   \
  Vec* sig;                                     \
  char* name

typedef struct Func {
  FUNC_HEADER;
} Func;

struct Closure {
  FUNC_HEADER;

  uint  nvars, nregs;

  // execution information
  Bin*   code;
  Vec*   vals;
  Env*   envt;
};

struct Native {
  FUNC_HEADER;

  rl_native_fn_t C_fn;
};

struct Primitive {
  FUNC_HEADER;

  OpCode label;
};

struct Generic {
  FUNC_HEADER;

  // data fields
  MMap*   cache;
  MTRoot* root;
};

struct MTRoot {
  HEADER;

  MTNode* f_root;
  MTNode* v_root;
};

struct MTNode {
  HEADER;

  
};

struct MTLeaf {
  HEADER;

  // bit fields
  word_t m_favor : 1;
  word_t m_union : 1;
  word_t m_var   : 1;

  // data fields
  Obj*  method;
};

// control type (reified continuation)
struct Cntl {
  HEADER;

  // data fields
  /* In case */
  Cntl* parent;

  /* Saved execution state */
  MVec* scpy;
  MVec* fcpy;

  /* Saved registers */
  Closure*  ex;
  uint16_t* ip;
  int       fs;
  int       co;
  int       ho;
  int       sp;
};

/* Globals */
extern Type ClosureType, NativeType, GenericType, ControlType, MTRootType, MTNodeType, MTLeafType;

/* APIs */
/* General function APIs */
char* fn_name(void* fn);
uint  fn_argc(void* fn);

/* Native APIs */
#define is_native(x) has_type(x, &NativeType)
#define as_native(x) ((Native*)as_obj(x))

/* Closure APIs */
#define is_cls(x) has_type(x, &ClosureType)
#define as_cls(x) ((Closure*)as_obj(x))

uint16_t* cls_ip(Closure* c);
size_t    cls_fs(Closure* c);
void      set_upv(Closure* c, UpVal* u, size_t i);

/* Cntl APIs */
#define is_cntl(x) has_type(x, &CntlType)
#define as_cntl(x) ((Cntl*)as_obj(x))

Cntl* mk_cntl(Cntl* p, IState* s);

/* Generic APIs */
Generic* mk_generic(char* name);
rl_err_t add_method(Generic* g, Vec* s, bool v, void* f);
rl_err_t get_method(Generic* g, Vec* s, Func** r);

#endif
