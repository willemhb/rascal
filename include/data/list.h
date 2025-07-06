#ifndef rl_data_list_h
#define rl_data_list_h

/**
 *
 * Rascal lists are based on tries. They are the go-to data structure
 * for storing sequenced data.
 *
 * They're a bit more complicated than linked lists, but
 * they have much better general performance characteristics
 * (at some point there will be a 'cons' type in the standard
 * library for handling the cases where a cons cell is actually
 * what's desired).
 *
 **/

// headers --------------------------------------------------------------------
#include "data/base.h"

// macros ---------------------------------------------------------------------
#define as_list(x) ((List*)as_obj(x))
#define is_list(x) has_type(x, EXP_LIST)
#define list_node_offset(n) ((n)->flags)

// C types --------------------------------------------------------------------
struct List {
  HEAD;

  size_t arity;
  ListNode* root;
  Expr tail[64];
};

struct ListNode {
  HEAD;

  Obj* children[64];
};

struct ListLeaf {
  HEAD;

  Expr slots[64];
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
