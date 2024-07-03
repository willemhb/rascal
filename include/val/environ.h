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
  word_t form    : 5;
  word_t literal : 1;

  // data fields
  Str*   nmspc;
  Str*   name;
  word_t idno;      // non-zero for gensyms
};

struct Env {
  HEADER;

  // bit fields
  word_t scope    : 3;
  word_t bound    : 1;
  word_t toplevel : 1; // special flag indicating the global namespace

  // data fields
  int     index;     // location within global namespace (-1 if this is a local environment)

  union {
    Sym*  name;      // name for this namespace (may be a function name or namespace name)
    char* _sname;    // aids static initialization
  };
  Env*    parent;    // the environment within which this environment was defined
  Env*    ns;        // shortcut to namespace-level parent
  EMap*   locals;    // local or namespace-scoped names
  EMap*   captured;  // names captured by upvalues
  Alist*  refs;      // sequential array of bindings
  UpVal** upvals;    // bound upvalues (if this is a bound local namespace)
};

struct Ref {
  HEADER;

  // bit fields
  word_t scope    : 3; // scope for this ref (determines where binding is stored)
  word_t final    : 1; // cannot be rebound once initialized
  word_t mm       : 1; // can have methods added
  word_t macro    : 1; // macro name
  word_t private  : 1; // whether this binding can be exported
  word_t dummy    : 1; // uninitialized forward reference

  // data fields
  Ref*  captures; // the binding captured by this binding (if any)
  Env*  environ;  // the environment in which the binding was *originally* created
  Sym*  name;     // name under which this binding was created in *original* environment
  int   offx;     // primary offset (location of binding)
  int   offy;     // secondary offset
  Type* tag;      // type constraint for this binding
  Val   val;      // just the value for this Ref (if namespace scoped), default initial value (if object scoped), or nothing (local/upvalue scoped)
};

struct UpVal {
  HEADER;

  // bit fields
  word_t closed : 1;

  // data fields
  UpVal* n_u;
  Proc* p;

  union {
    size_t l;
    Val    v;
  };
};

/* Globals */
// types
extern Type SymType, EnvType, RefType, UpValType;

// global environment
extern Str* GlobalNs;

/* APIs */
/* General utilities */
bool is_form_name(char* n);

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
Sym* unqualify(Sym* s);
Sym* cstr_qualify(Sym* s, char* ns);
Sym* str_qualify(Sym* s, Str* ns);
Sym* sym_qualify(Sym* s, Sym* ns);

/* Env APIs */
#define is_env(x) has_type(x, &EnvType)
#define as_env(x) ((Env*)as_obj(x))

size_t lref_cnt(Env* p);
size_t uref_cnt(Env* p);
Ref*   env_ref(Env* e, size_t o);
Env*   mk_env(Scope s, Env* p, Sym* n, int i, bool b);
void   init_gns(State* s, Env* g);
Env*   bind_env(Env* t);
bool   env_get(Env* e, Sym* n, int c, Ref** r);
bool   env_put(Env* e, Sym* n, Ref** r);
bool   has_ref(Env* e, int o);

/* Ref APIs */
#define is_ref(x)   has_type(x, &RefType)
#define as_ref(x)   ((Ref*)as_obj(x))

Ref* mk_ref(Scope s, Env* e, Ref* c, Sym* n);
bool is_lupv(Ref* r);
bool is_ns_ref(Ref* r);

/* UpVal APIs */
#define is_upval(x) has_type(x, &UpValType)
#define as_upval(x) ((UpVal*)as_obj(x))

UpVal* mk_upv(UpVal* n_u, Val* l);
Val*   dr_upv(UpVal* u);
UpVal* open_upv(UpVal** o, Val* l);
void   close_upv(UpVal** o, UpVal* u);
void   close_upvs(UpVal** o, Val* l);

// utilities for working with symbols and environments
#define define(n, x, e)                                     \
  generic((n),                                              \
          char*:generic((x),                                \
                        Val:cstr_define_val,                \
                        default:cstr_define_obj),           \
          Sym*:generic((x),                                 \
                       Val:sym_define_val,                  \
                       default:sym_define_obj))(n, x, e)

// define methods
size_t cstr_define_val(char* n, Val x, Env* e);
size_t cstr_define_obj(char* n, void* x, Env* e);
size_t sym_define_val(Sym* n, Val i, Env* e);
size_t sym_define_obj(Sym* n, void* x, Env* e);

#endif
