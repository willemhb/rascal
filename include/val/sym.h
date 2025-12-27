#ifndef rl_sym_h
#define rl_sym_h

#include "val/val.h"
#include "val/str.h"

struct Sym {
  HEAD;
  Str*   val;
  hash_t hash;
};

// symbol API
Sym* mk_sym(RlState* rls, char* cs);
Sym* mk_sym_s(RlState* rls, char* cs);
Sym* mk_module_name_s(RlState* rls, char* fname);
bool sym_val_eql(Sym* s, char* v);

// convenience macros
#define as_sym(x)         ((Sym*)as_obj(x))
#define as_sym_s(rls, x)  ((Sym*)as_obj_s(rls, &SymType, x))
#define is_keyword(s)     (*(s)->val->val == ':')
#define sym_val(s)        ((s)->val->val)

#endif
