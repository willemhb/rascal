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

  Sym*   n;   // namespace name
  size64 id;  // equivalent to offset in global Ns (global Ns itself has offset 0)
  Table* rs;  // namespace references
  Alist* vs;  // namespace values
};

struct Env {
  HEADER;

  Env*   p;   // enclosing environment

  Table* l;   // local references
  Table* u;   // captured references
};

struct Ref {
  HEADER;

  RefType type;    // specifies where to look for the value
  Ns*     ns;      // corresponding namespace (if applicable)
  Ref*    capture; // For upvalues, this is the reference they capture (might be a local or another upvalue)
  Sym*    name;    // fully qualified symbol name (note that the reference might b)
  size64  offset;  // offset of the value in a corresponding array (might be the stack, bound upvalues, or a namespace)
};

struct Upv {
  HEADER;

  bool8 open;
  Upv*  next;

  union {
    Val* loc;      // stack reference for open upvalues
    Val  val;      // immediate reference for closed upvalues
  };
};

/* External APIs */
// Sym API
#define is_sym(x) has_type(x, T_SYM)
#define as_sym(x) ((Sym*)as_obj(x))

Sym* new_sym(Str* n, Str* ns, bool lit, bool gs);
Sym* get_sym(char* n, char* ns, bool gs);

// Ns API


// Upv API
#define is_upv(x) has_type(x, T_UPV)
#define as_upv(x) ((Upv*)as_obj(x))

static inline Val* dr_upv(Upv* u) {
  return u->open ? u->loc : &u->val;
}

Upv* new_upv(Val* l, Upv* n);
Upv* get_upv(Val* l);
Upv* close_upv(Upv* u);

#endif
