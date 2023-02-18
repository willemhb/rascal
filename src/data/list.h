#ifndef data_list_h
#define data_list_h

#include "base/object.h"

/* C types */
struct List {
  Obj obj;

  Val   head;
  List *tail;
};

/* globals */
extern List EmptyList;

/* API */
bool  is_list(Val x);
List* as_list(Val x);
Val   mk_list(List* l);

#endif
