#ifndef rl_val_env_h
#define rl_val_env_h

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
  MMap* rs;   // namespace references
  MVec* vs;   // namespace values
};

struct Env {
  HEADER;

  Env*  p;   // enclosing environment
  MMap* l;   // local references
  MMap* u;   // captured references
};

struct Ref {
  HEADER;

  RefType rtype;   // 
  Ns*     ns;      // namespace in which the ref was created
  Ref*    capture; // For upvalues, this is the reference they capture (might be a local or another upvalue)
  Sym*    name;    // fully qualified symbol name (note that the reference might be registered )
  size64  offset;  // offset of the value in a corresponding array (might be the stack, bound upvalues, or a namespace)
};

struct Upv {
  HEADER;

  bool  open;
  Upv*  next;

  union {
    Val* loc;      // stack reference for open upvalues
    Val  val;      // immediate reference for closed upvalues
  };
};

/* External APIs */
// Sym API
#define is_sym(x) has_vtype(x, T_SYM)
#define as_sym(x) ((Sym*)as_obj(x))

Sym* new_sym(Str* n, Str* ns, bool lit, bool gs);
Sym* get_sym(char* n, char* ns, bool gs);

// Ns API
#define is_ns(x) has_vtype(x, T_NS)
#define as_ns(x) ((Ns*)as_obj(x))

Ns*  new_ns(void);
Ref* ns_lookup(Ns* ns, Sym* n);
Ns*  mk_ns(State* vm, void* n, bool tl);
Ref* def_global(Ns* ns, void* n);

// Ref API
#define is_ref(x) has_vtype(x, T_REF)
#define as_ref(x) ((Ref*)as_obj(x))

Ref* init_ref(Ref* r, Val x);

// Upv API
#define is_upv(x) has_vtype(x, T_UPV)
#define as_upv(x) ((Upv*)as_obj(x))

static inline Val* dr_upv(Upv* u) {
  return u->open ? u->loc : &u->val;
}

Upv* new_upv(Val* l, Upv* n);
Upv* close_upv(Upv* u);

#endif
