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

struct Pair {
  HEADER;
  Value car;
  Value cdr;
};

struct MutList {
  HEADER;
  List* tail;
  Value head;
};

struct MutPair {
  HEADER;
  Value car;
  Value cdr;
};

/* globals */
extern Type ListType;
extern List EmptyList;

/* External APIs */
/* List API */
#define is_list(x) has_type(x, &ListType)
#define as_list(x) as(List*, untag48, x)

List* mk_list(size_t n, Value* a);
List* cons(Value head, List* tail);
List* cons_n(size_t n, Value* a);

// accessors
Value list_ref(List* xs, size_t n);


// sequence utilities
List* list_cat(List* a, List* b);
List* list_rev(List* xs);

#endif
