#ifndef rl_val_list_h
#define rl_val_list_h

#include "val/object.h"

/* Simple persistent linked list type.

   Unlike in traditional lisp dialects, rascal lists are proper (no dotted lists). */

/* C types */
struct List {
  HEADER;
  List*  tail;
  size_t arity;
  Value  head;
};

/* globals */
extern struct Type ListType;
extern struct List EmptyList;

/* external API */
// casts & predicates
bool  val_is_list(Value x);
bool  obj_is_list(Obj* obj);
List* as_list(Value x);

// constructors
List* mk_list(size_t n, Value* a);
List* cons(Value head, List* tail);
List* cons_n(size_t n, Value* a);

// accessors
Value list_ref(List* xs, size_t n);

// sequence utilities
List* list_cat(List* a, List* b);
List* list_rev(List* xs);

#endif
