#ifndef rl_data_types_upv_h
#define rl_data_types_upv_h

/* Special type for indirecting captured references, used to implement closures. */
// headers --------------------------------------------------------------------
#include "common.h"
#include "data/expr.h"
#include "data/obj.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct UpVal {
  HEAD;

  UpVal* next;
  bool   closed;

  union {
    Expr  val;
    Expr* loc;
  };
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
UpVal* mk_upval(UpVal* next, Expr* loc);
Expr*  deref(UpVal* upv);

// initialization -------------------------------------------------------------
void toplevel_init_data_type_upv(void);

#endif
