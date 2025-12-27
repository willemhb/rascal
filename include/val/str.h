#ifndef rl_str_h
#define rl_str_h

#include "val/val.h"

struct Str {
  HEAD;
  char*  val;
  size_t count;
  hash_t hash;
};

// string API
Str* new_str(RlState* rls, char* cs, hash_t h, bool interned);
Str* mk_str(RlState* rls, char* cs);
Str* mk_str_s(RlState* rls, char* cs);

// macros
#define as_str(s)        ((Str*)as_obj(s))
#define as_str_s(rls, s) ((Str*)as_obj_s(rls, &StrType, s))
#define str_val(s)       ((s)->val)
#define is_interned(s)   ((s)->flags == true)

#endif
