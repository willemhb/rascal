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

#endif
