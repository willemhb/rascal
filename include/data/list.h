#ifndef rl_data_list_h
#define rl_data_list_h

/**
 *
 * Rascal lists are based on linked lists.
 *
 * In the future I would like to supply a trie-based list, but the implementation
 * is more complicated and it requires significantly more bells and whistles to
 * operate on efficiently without a stateful iteration construct.
 *
 **/

// headers --------------------------------------------------------------------
#include "data/base.h"

// macros ---------------------------------------------------------------------
#define as_list(x) ((List*)as_obj(x))
#define is_list(x) has_type(x, EXP_LIST)

// C types --------------------------------------------------------------------
struct List {
  HEAD;

  size_t arity;
  Expr* head;
  List* tail;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
List*  as_list_s(char* f, Expr x);
List*  empty_list(void);
List*  mk_list(size_t n, Expr* xs);
Expr   list_ref(List* xs, size_t n);
List*  list_add(List* xs, Expr x);
List*  list_pop(List* xs);
List*  list_set(List* xs, size_t o, Expr x);

// initialization -------------------------------------------------------------
void toplevel_init_data_list(void);

#endif
