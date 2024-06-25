#ifndef rl_function_h
#define rl_function_h

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */
struct Closure {
  HEADER;
  // bit fields

  // data fields
  Bin* code;
  Vec* vals;
  Env* envt;
};

struct Native {
  HEADER;

  rl_native_fn_t C_fn;
};

struct Generic {
  HEADER;

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
  size_t argc;
  Vec*   sig;
  Obj*   method;
};

// control type (reified continuation)
struct Cntl {
  HEADER;

  // data fields
  IFrame* frames;   // preserved stack frames
  size_t  fcount;   // frame count
  MVec*   sbuffer;  // buffer for stack values
};

/* Globals */
extern Type ClosureType, NativeType, GenericType, ControlType, MTRootType, MTNodeType, MTLeafType;

/* APIs */
/* Closure APIs */
#define is_cls(x) has_type(x, &ClosureType)
#define as_cls(x) ((Closure*)as_obj(x))

size_t    cls_esize(Closure* c);
uint16_t* cls_ip(Closure* c);

/* Generic APIs */
Generic* mk_generic(void);

rl_err_t add_method(Generic* g, Vec* s, bool v, void* f);
rl_err_t get_method(Generic* g, Vec* s, MTLeaf** r);

#endif
