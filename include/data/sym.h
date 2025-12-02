#ifndef rl_data_sym_h
#define rl_data_sym_h

/* First-class identifier type. A Lisp classic! */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"
#include "data/str.h"

// macros ---------------------------------------------------------------------
#define as_sym(x)     ((Sym*)as_obj(x))
#define is_sym(x)     has_type(x, EXP_SYM)

// C types --------------------------------------------------------------------
struct Sym {
  HEAD;

  Str*   val;
  hash_t hash;
  Sym*   left;
  Sym*   right;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Sym* as_sym_s(char* f, Expr x);
Sym* mk_sym(char* cs);
bool sym_val_eql(Sym* s, char* v);

// initialization -------------------------------------------------------------
void toplevel_init_data_sym(void);

#endif
