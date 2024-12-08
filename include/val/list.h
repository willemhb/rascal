#ifndef rl_val_list_h
#define rl_val_list_h

#include "val/object.h"

/*
 * Classic lisp list, with caveats:
 *
 * 1) Rascal lists are immutable.
 * 2) Rascal lists are always proper lists.
 *
 * For more traditional cons cells, see val/pair.h.
 */

/* C types */
struct List {
  HEADER;

  Val    head;
  size64 cnt;
  List*  tail;
};

/* Globals */
extern VTable ListVt;
extern List   EmptyList;

/* API */
#define is_list(x) has_type(x, T_LIST)
#define as_list(x) ((List*)as_obj(x))

#define mk_list(x, y)                           \
  generic((y),                                  \
          Val*:mk_listn,                        \
          List*:mk_list2)(x, y)

List* mk_list2(Val hd, List* tl);
List* mk_listn(size32 n, Val* d);
Val   list_ref(List* xs, size64 n);
Val*  push_list(Proc* p, List* xs);

/* Initialization */
void rl_toplevel_init_list(void);

#endif
