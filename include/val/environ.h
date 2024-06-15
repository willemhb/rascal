#ifndef rl_val_environ_h
#define rl_val_environ_h

#include "val/object.h"

/* Types and APIs, and globals for the rascal namespace and environment system. */

/* C types */
typedef enum Scope {
  LOCAL_SCOPE,     // value is on stack
  UPVALUE_SCOPE,   // value is indirected through an upvalue
  NAMESPACE_SCOPE, // value is stored in the current namespace environ
  STRUCT_SCOPE,    // value is stored in struct object
  RECORD_SCOPE,    // value is stored in record object
} Scope;

// user identifier types
struct Sym {
  HEADER;

  // bit fields
  word_t literal : 1;

  // identifier info
  Str*   nmspc;
  Str*   name;
  word_t idno;      // non-zero for gensyms
};

struct Env {
  HEADER;

  // bit fields
  word_t scope    : 3;
  word_t bound    : 1;
  word_t captured : 1;

  // data fields
  Sym*  name;      // Name for this Env object (may be a namespace, function, or type)
  Env*  parent;    // the environment within which this environment was defined
  Env*  template;  // the unbound environment a bound environment was cloned from
  EMap* locals;
  EMap* nonlocals;

  union {
    Alist* upvals;
    MVec*  values;
  };
};

struct Ref {
  HEADER;

  // bit fields
  word_t scope  : 3;
  word_t final  : 1;
  word_t inited : 1;
  word_t mm     : 1; // can have methods added
  word_t macro  : 1; // macro name

  // data fields
  Ref*   captures; // the binding captured by this binding (if any)
  Env*   environ;  // the environment in which the binding was *originally* created
  Sym*   name;     // name under which this binding was created in *original* environment
  size_t offset;   // location (may be on stack, in upvalues, or directly in environment)
  Type*  tag;      // type constraint for this binding
  Val    init;     // default initval (only used for object scopes)
};

struct UpVal {
  HEADER;

  // bit fields
  word_t closed : 1;

  // data fields
  UpVal* next_upv;

  union {
    Val* location;
    Val  value;
  };
};

/* Globals */
// types
extern Type SymType, EnvType, RefType, UpValType;

// global environment
extern Env Globals;
extern Str* GlobalNs;

/* APIs */
/* Sym API */
#define is_sym(x) has_type(x, &SymType)
#define as_sym(x) ((Sym*)as_obj(x))

static inline bool is_gs(Sym* s) {
  return s->idno > 0;
}

static inline Str* get_ns(Sym* s) {
  return s->nmspc ? : GlobalNs;
}

#define qualify(s, ns)                          \
  generic((ns),                                 \
          char*:cstr_qualify,                   \
          Str*:str_qualify,                     \
          Sym*:sym_qualify)(s, ns)

#define mk_sym(n, ns, gs)                       \
  generic((n),                                  \
          char*:c_mk_sym,                       \
          Str*:s_mk_sym)(n, ns, gs)

#define sn_eq(x, c)                             \
  generic((x),                                  \
          Sym*:s_sn_eq,                         \
          Val: v_sn_eq)(x, c)

Sym* c_mk_sym(char* n, char* ns, bool gs);
Sym* s_mk_sym(Str* n, Str* ns, bool gs);

bool s_sn_eq(Sym* s, const char* n);
bool v_sn_eq(Val x, const char* n);

// qualify methods
Sym* cstr_qualify(Sym* s, char* ns);
Sym* str_qualify(Sym* s, Str* ns);
Sym* sym_qualify(Sym* s, Sym* ns);

/* Ref APIs */
#define is_ref(x)   has_type(x, &RefType)
#define as_ref(x)   ((Ref*)as_obj(x))

Ref* mk_ref(Scope* scope, Env* environ);

/* UpVal APIs */
#define is_upval(x) has_type(x, &UpValType)
#define as_upval(x) ((UpVal*)as_obj(x))

static inline Val* deref_upval(UpVal* upv) {
  if ( upv->closed )
    return &upv->value;

  return upv->location;
}

// utilities for working with symbols and environments
#define define(n, v, e)                         \
  generic((n),                                  \
          char*:cstr_define,                    \
          Sym*:sym_define)(n, v, e)

// define methods
size_t cstr_define(char* n, Val i, Env* e);
size_t sym_define(Sym* n, Val i, Env* e);

#endif
