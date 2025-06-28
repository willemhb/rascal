#ifndef rl_data_types_alist_h
#define rl_data_types_alist_h

/* Internal mutable array type. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/expr.h"
#include "data/obj.h"
#include "data/array.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
// wrapper around Exprs object
struct Alist {
  HEAD;

  Exprs exprs;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Alist* mk_alist(void);
void   free_alist(void* ptr);
int    alist_push(Alist* a, Expr x);
Expr   alist_pop(Alist* a);
Expr   alist_get(Alist* a, int n);

// initialization -------------------------------------------------------------
void toplevel_init_data_type_alist(void);

#endif
