#ifndef rl_data_sym_h
#define rl_data_sym_h

/* First-class identifier type. A Lisp classic! */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define as_sym(x) ((Sym*)as_obj(x))
#define is_sym(x) has_type(x, EXP_SYM)

// C types --------------------------------------------------------------------
struct Sym {
  HEAD;

  char* val;
  idno_t idno; /* gensym counter, 0 if this is an interned symbol */
  hash_t hash;
};

// globals --------------------------------------------------------------------
extern Type SymType;

// function prototypes --------------------------------------------------------
Sym* as_sym_s(char* f, Expr x);
Sym* new_sym(char* cs, bool intern, hash_t h);

// interface functions
Status iget_sym(RascalState* s, char* name, bool intern);

// initialization -------------------------------------------------------------
void toplevel_init_data_sym(void);

#endif
