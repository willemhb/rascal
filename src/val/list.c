#include "val/list.h"

#include "vm/heap.h"

/* Internal APIs */
void init_list(List* x, size_t n, Val h, List* t) {
  x->head = h;
  x->cnt  = n;
  x->tail = t;
}

/* External APIs */
/* List API */

List* mk_list(size_t n, Val* a) {
  List* o;
  
  if ( n == 0 )
    o = &EmptyList;

  else if ( n == 1 )
    o = c_list(a[1], &EmptyList);

  else {
    o = allocate(n*sizeof(List), true);

    for ( size_t i=0; i<n; i++ ) {
      List* t = i + 1 == n ? &EmptyList : o+i+1;

      init_obj(&ListType, (Obj*)(o+i));
      init_list(o+i, n-i, a[i], t);
    }
  }

  return o;
}

List* c_list(Val h, List* t) {
  List* o = new_obj(&ListType);

  init_list(o, t->cnt+1, h, t);

  return o;
}

Val list_ref(List* x, size_t n) {
  assert(n < x->cnt);

  while ( n-- )
    x = x->tail;

  return x->head;
}

/* Pair API */
Pair* mk_pair(Val a, Val d) {
  Pair* p = new_obj(&PairType);
  p->car  = a;
  p->cdr  = d;

  return p;
}

/* MList API */


/* MPair API */
MPair* mk_mpair(Val a, Val d) {
  MPair* o = new_obj(&MPairType);

  o->car = a;
  o->cdr = d;

  return o;
}
