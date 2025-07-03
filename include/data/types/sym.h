#ifndef rl_data_types_sym_h
#define rl_data_types_sym_h

/* First-class identifier type. A Lisp classic! */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define as_sym(x)     ((Sym*)as_obj(x))
#define is_keyword(s) (*(s)->val->val == ':')
#define is_sym(x)     has_type(x, EXP_SYM)

// C types --------------------------------------------------------------------
struct Sym {
  HEAD;

  Str*   val;
  hash_t hash;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Sym* as_sym_s(char* f, Expr x);
Sym* mk_sym(char* cs);
bool sym_val_eql(Sym* s, char* v);

// initialization -------------------------------------------------------------
void toplevel_init_data_type_sym(void);

#endif
