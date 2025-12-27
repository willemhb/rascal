#include "val/sym.h"
#include "val/port.h"
#include "vm.h"
#include "util/util.h"

// forward declarations
void print_sym(Port* ios, Expr x);
hash_t hash_sym(Expr x);
bool egal_syms(Expr x, Expr y);
void trace_sym(RlState* rls, void* ptr);

// Type object
Type SymType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_SYM,
  .obsize   = sizeof(Sym),
  .trace_fn = trace_sym,
  .print_fn = print_sym,
  .hash_fn  = hash_sym,
  .egal_fn  = egal_syms
};

// symbol API
Sym* mk_sym(RlState* rls, char* val) {
  StackRef top = rls->s_top;
  Sym* s = mk_obj_s(rls, &SymType, 0);
  s->val  = mk_str(rls, val);
  s->hash = hash_word(s->val->hash); // just munge the string hash
  rls->s_top = top;
  return s;
}

Sym* mk_sym_s(RlState* rls, char* val) {
  Sym* out = mk_sym(rls, val);
  stack_push(rls, tag_obj(out));
  return out;
}

bool sym_val_eql(Sym* s, char* v) {
  return streq(s->val->val, v);
}

void trace_sym(RlState* rls, void* ptr) {
  Sym* s = ptr;

  mark_obj(rls, s->val);
}

void print_sym(Port* ios, Expr x) {
  Sym* s = as_sym(x);

  pprintf(ios, "%s", s->val->val);
}

hash_t hash_sym(Expr x) {
  Sym* s = as_sym(x);

  return s->hash;
}

bool egal_syms(Expr x, Expr y) {
  Sym* sx = as_sym(x), * sy = as_sym(y);

  return sx->val == sy->val;
}
