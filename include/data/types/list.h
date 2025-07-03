#ifndef rl_data_types_list_h
#define rl_data_types_list_h

/* The great Lisp classic.

   As in Clojure, all Rascal lists are proper.

   Nobody really gives a fuck about reader graphs. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/expr.h"
#include "data/obj.h"

// macros ---------------------------------------------------------------------
#define as_list(x) ((List*)as_obj(x))
#define is_list(x) has_type(x, EXP_LIST)


// C types --------------------------------------------------------------------
struct List {
  HEAD;

  Expr   head;
  List*  tail;
  size_t count;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
List*  as_list_s(char* f, Expr x);
List*  empty_list(void);
List*  mk_list(size_t n, Expr* xs);
List*  cons(Expr hd, List* tl);
Expr   list_ref(List* xs, int n);


// initialization -------------------------------------------------------------

#endif
