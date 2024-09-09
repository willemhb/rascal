#ifndef rl_val_environ_h
#define rl_val_environ_h

#include "val/object.h"

/* Types and APIs, and globals for the rascal namespace and environment system. */

/* C types */
typedef enum Scope {
  S_LOCAL,
  S_UPVAL,
  // S_FILE,
  S_GLOBAL
} Scope;

// user identifier types
struct Sym {
  HEADER;

  flags_t labl : 31; // arbitrary internal label
  flags_t litp :  1; // if true, this symbol is always self-evaluating

  Str*    name;
  word_t  idno;      // non-zero for gensyms
};

struct Env {
  HEADER;

  Scope  scope;

  Env*   nxt;           // parent environment
  Table* vars, * caps;  // tables for local variables and variables captured from parent environment (upvalues)
  Alist* binds;         // 
};

struct Ref {
  HEADER;

  flags_t tag    : 27; // type constraint
  flags_t scope  :  2; // 
  flags_t initp  :  1; // 
  flags_t macrop :  1; // 
  flags_t finalp :  1; // 

  arity_t offset;      // 

  Sym*    name;        // source name for this ref
  Val     val;         // value ()
  Ref*    caps;
  Str*    doc;
};

struct UpVal {
  HEADER;

  bool   open;

  UpVal* nxt;

  union {
    size_t l;
    Val    v;
  };
};

/* APIs */
/* General utilities */
bool is_form_name(char* n);

/* Sym API */
#define is_sym(x) has_type(x, T_SYM)
#define as_sym(x) ((Sym*)as_obj(x))

static inline bool is_gs(Sym* s) {
  return s->idno > 0;
}

#define mk_sym(n, ns, gs)                       \
  generic((n),                                  \
          char*:c_mk_sym,                       \
          Str*:s_mk_sym)(n, ns, gs)

#define sn_eq(x, c)                             \
  generic((x),                                  \
          Sym*:s_sn_eq,                         \
          Val: v_sn_eq)(x, c)

Sym* c_mk_sym(char* n, bool gs);
Sym* s_mk_sym(Str* n, bool gs);

bool s_sn_eq(Sym* s, const char* n);
bool v_sn_eq(Val x, const char* n);

/* UpVal APIs */
#define is_upval(x) has_type(x, T_UPVAL)
#define as_upval(x) ((UpVal*)as_obj(x))

UpVal* mk_upv(UpVal* n_u, Val* l);
Val*   dr_upv(UpVal* u);
UpVal* open_upv(UpVal** o, Val* l);
void   close_upv(UpVal** o, UpVal* u);
void   close_upvs(UpVal** o, Val* l);

#endif
