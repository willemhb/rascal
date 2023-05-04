#ifndef list_h
#define list_h

#include "data/object.h"

// C types
struct list {
  HEADER;
  value_t head;
  list_t* tail;
  usize   arity;
};

// API & utilities
#define is_list(x) rl_isa(x, LIST)
#define as_list(x) rl_asa(x, WVMASK, list_t*)

list_t* list(usize n, value_t* args);
list_t* cons(value_t hd, list_t* tl);

// globals
extern list_t EmptyList;

#endif
