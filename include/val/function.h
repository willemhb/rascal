#ifndef rl_val_function_h
#define rl_val_function_h

#include "labels.h"

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */
#define FUNC_HEADER                                                   \
  HEADER;                                                             \
  /* bit fields */                                                    \
  word_t ft      : 3;                                                 \
  word_t framec  : 3;                                                 \
  word_t va      : 1;                                                 \
  word_t comp    : 1;                                                 \
  word_t m_union : 1;                                                 \
  word_t m_favor : 1;                                                 \
  /* data fields */                                                   \
  uint argc, lvarc;                                                   \
  Vec* sig;                                                           \
  /* _sname field exists to aid static initialization */              \
  union {                                                             \
    char* _name;                                                      \
    Sym*  name;                                                       \
  }

typedef struct Func {
  FUNC_HEADER;
} Func;

struct Closure {
  FUNC_HEADER;

  // execution information
  union {
    MBin* c_code;
    Bin*  code;
  };
  
  union {
    MVec* c_vals;
    Bin*  vals;
  };

  Env*   envt;
};

struct Native {
  FUNC_HEADER;

  NativeFn C_fn;
};

struct Primitive {
  FUNC_HEADER;

  Label label;
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

// control type (reified continuation)
struct Cntl {
  HEADER;

  // data fields
  /* If `raise` is called from within a `hndl` body, this is the Cntl object that was in scope at that time */
  Cntl* parent;

  /* copy of stack values */
  MVec* scpy;

  /* Saved registers */
  Closure*  ex;
  short*    ip;
  size_t    fs;
  size_t    cp;
  size_t    hp;
};

/* Globals */
extern Type ClosureType, NativeType, GenericType, ControlType, MTRootType, MTNodeType, MTLeafType;

/* APIs */
/* General function APIs */
#define as_func(x) ((Func*)as_obj(x))

char*  fn_name(void* fn);
size_t fn_argc(void* fn);
size_t fn_lvarc(void* fn);
size_t fn_framec(void* fn);
size_t fn_fsize(void* fn);

/* Native APIs */
#define is_native(x) has_type(x, &NativeType)
#define as_native(x) ((Native*)as_obj(x))

/* Closure APIs */
#define is_cls(x) has_type(x, &ClosureType)
#define as_cls(x) ((Closure*)as_obj(x))

short*  cls_ip(Closure* c);

/* Cntl APIs */
#define is_cntl(x) has_type(x, &CntlType)
#define as_cntl(x) ((Cntl*)as_obj(x))

Cntl* mk_cntl(Cntl* p, RlState* s);

/* Generic APIs */
Generic* mk_generic(char* name);
Error add_method(Generic* g, Vec* s, bool v, void* f);
Error get_method(Generic* g, Vec* s, Func** r);

#endif
