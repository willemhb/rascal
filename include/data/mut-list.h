#ifndef rl_data_mut_list_h
#define rl_data_mut_list_h

/* Internal mutable array type. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
// wrapper around Exprs object
struct MutList {
  HEAD;

  Exprs exprs;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
MutList* mk_mut_list(void);
void     free_mut_list(void* ptr);
int      mut_list_push(MutList* a, Expr x);
Expr     mut_list_pop(MutList* a);
Expr     mut_list_ref(MutList* a, int n);

// initialization -------------------------------------------------------------
void toplevel_init_data_mut_list(void);

#endif
