#ifndef rl_val_environ_h
#define rl_val_environ_h

#include "val/object.h"

/* Reference and environment types go here */
// first class name type
struct Sym {
  HEADER;

  bool   literal;
  uint64 idno;

  Str*   ns;
  Str*   name;
};

struct Env {
  HEADER;

  Env*   parent;

  Table* locals;
  Table* upvals;
  Table* module;
};

/* External APIs */
#define is_sym(x) has_type(x, T_SYM)
#define as_sym(x) ((Sym*)as_obj(x))

Sym* new_sym(Str* n, Str* ns, bool lit, bool gs);
Sym* get_sym(char* n, char* ns, bool gs);

#endif
