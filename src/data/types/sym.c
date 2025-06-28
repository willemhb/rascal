/* First-class identifier type. A Lisp classic! */
// headers --------------------------------------------------------------------
#include "data/types/sym.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void   print_sym(Port* ios, Expr x);
hash_t hash_sym(Expr x);
bool   egal_syms(Expr x, Expr y);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
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

// external -------------------------------------------------------------------
Sym* as_sym_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_SYM, "%s wanted a sym, got %s", f, Types[t].name);

  return as_sym(x);
}

Sym* mk_sym(char* val) {
  Sym* s  = mk_obj(EXP_SYM, 0); preserve(1, tag_obj(s));
  s->val  = mk_str(val);
  s->hash = hash_word(s->val->hash); // just munge the string hash

  return s;
}

bool sym_val_eql(Sym* s, char* v) {
  return streq(s->val->val, v);
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_sym(void) {
     Types[EXP_SYM] = (ExpTypeInfo) {
    .type     = EXP_SYM,
    .name     = "sym",
    .obsize   = sizeof(Sym),
    .print_fn = print_sym,
    .hash_fn  = hash_sym,
    .egal_fn  = egal_syms
     };
}
