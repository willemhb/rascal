#ifndef rl_sym_h
#define rl_sym_h

#include "val/val.h"
#include "val/str.h"

struct Sym {
  HEAD;
  Str*      val;
  uintptr_t idno; // for gensyms; 0  for 'interned' syms
};

// symbol API
Sym* mk_sym(RlState* rls, char* cs);
Sym* mk_sym_s(RlState* rls, char* cs);
Sym* mk_gensym(RlState* rls, char* cs);
Sym* mk_gensym_s(RlState* rls, char* cs);
bool sym_val_eql(Sym* s, char* v);

// convenience macros
#define as_sym(x)         ((Sym*)as_obj(x))
#define as_sym_s(rls, x)  ((Sym*)as_obj_s(rls, &SymType, x))
#define is_keyword(s)     (*(s)->val->val == ':')
#define is_gensym(s)      ((s)->idno != 0)
#define sym_val(s)        ((s)->val->val)

#endif
