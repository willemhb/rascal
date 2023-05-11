#include "data/list.h"

#include "runtime/memory.h"

// local utilities
static void init_list(list_t* ls, value_t hd, list_t* tl) {
  INIT_HEADER(&ls->obj, LIST, FROZEN);

  ls->head  = hd;
  ls->tail  = tl;
  ls->arity = tl->arity + 1;
}

// API & utilities
list_t* list(usize n, value_t* args) {
  if (n == 0)
    return &EmptyList;

  if (n == 1)
    return cons(*args, &EmptyList);

  list_t* out = allocate(sizeof(list_t)*n), * space = &out[n-1];

  for (usize i = n; i > 0; i--) {
    init_list(space, args[n-1], i == n ? &EmptyList : space+1);
    space--;
  }

  return out;
}

list_t* cons(value_t hd, list_t* tl) {
  list_t* out = allocate(sizeof(list_t));
  init_list(out, hd, tl);
  return out;
}

// core methods
usize list_size(void* obj) {
  (void)obj;

  return sizeof(list_t);
}

void trace_list(void* obj) {
  list_t* ls = obj;

  mark_value(ls->head);
  mark_object(ls->tail);
}


