#ifndef rl_val_environ_h
#define rl_val_environ_h

#include "labels.h"

#include "val/object.h"

/* Reference and environment types go here */
// first class name type
struct Sym {
  HEADER;

  bool   lit;
  uint64 id;

  Str*   ns;
  Str*   n;
};

struct Ns {
  HEADER;

  Sym*   n;  // namespace name
  Table* rs; // namespace references
  Alist* vs; // namespace values
};

struct Env {
  HEADER;

  Env*   p;   // enclosing environment

  Table* l;   // local references
  Table* u;   // captured references
  Table* n;   // namespace referens
  Table* g;   // global namespace references
};

struct Ref {
  HEADER;

  RefType type;    // specifies where to look for the value
  Ref*    capture; // For upvalues, this is the reference they capture (might be a local or another upvalue)
  Sym*    name;    // fully qualified symbol name (note that the reference might b)
  size64  offset;  // offset of the value in a corresponding array (might be the stack, bound upvalues, or a namespace)
};

/* External APIs */
#define is_sym(x) has_type(x, T_SYM)
#define as_sym(x) ((Sym*)as_obj(x))

Sym* new_sym(Str* n, Str* ns, bool lit, bool gs);
Sym* get_sym(char* n, char* ns, bool gs);

#endif
