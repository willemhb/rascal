#include "val/list.h"
#include "val/array.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/hash.h"

/* Globals */
List EmptyList = {
  .tag        = T_LIST,
  .nosweep    = true,
  .persistent = true,
  .gray       = true,
  .head       = NUL,
  .cnt        = 0,
  .tail       = &EmptyList,
};

/* Internal APIs */
void init_list(List* l, Val h, size64 c, List* t) {
  l->head = h;
  l->cnt  = c;
  l->tail = t;
}

/* Runtime APIs */
void trace_list(State* vm, void* x) {
  List* l = x;

  mark(vm, l->head);
  mark(vm, l->tail);
}

hash64 hash_list(Val x) {
  List* lx = as_list(x);

  // should never be called on an empty list
  assert(lx->cnt == 0);

  // mix hashes of values (rl_hash handles mixing with type hash)
  hash64 h = 0;

  for ( ; lx->cnt; lx=lx->tail )
    h = h ? mix_hashes(h, rl_hash(lx->head)) : rl_hash(lx->head);

  return h;
}

bool egal_lists(Val x, Val y) {
  List* xl = as_list(x), * yl = as_list(y);

  bool out = xl->cnt == yl->cnt;

  for ( ; out && xl->cnt; xl=xl->tail, yl=yl->tail )
    out = rl_egal(xl->head, yl->head);

  return out;
}

int order_lists(Val x, Val y);

/* External APIs */
// List API
List* mk_list2(Val hd, List* tl) {
  List* l = new_obj(&Vm, T_LIST, MF_PERSISTENT);

  init_list(l, hd, tl->cnt+1, tl);

  return l;
}

List* mk_listn(size32 n, Val* d) {
  if ( n == 0 )
    return &EmptyList;

  else if ( n == 1 )
    return mk_list(d[0], &EmptyList);

  else {
    // allocate together
    List* out = rl_alloc(&Vm, n*sizeof(List));
    List* spc = out;

    // initialize lists
    for ( size64 i=0, j=n; i < n; i++, j--) {
      List* nxt = i + 1 == n ? &EmptyList : spc+1;

      init_obj(&Vm, (Obj*)spc, T_LIST, MF_PERSISTENT);
      init_list(spc, d[i], j, nxt);
    }

    return out;
  }
}

Val list_ref(List* xs, size64 n) {
  assert(n < xs->cnt);

  while ( n-- )
    xs = xs->tail;

  return xs->head;
}

void rl_toplevel_init_list(void) {
  // compute empty list hash
  EmptyList.hash = mix_hashes(hash_word(T_LIST), hash_pointer(&EmptyList));
}
