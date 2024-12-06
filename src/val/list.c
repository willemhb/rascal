#include "val/list.h"
#include "val/alist.h"
#include "val/text.h"

#include "lang/compare.h"
#include "lang/print.h"

#include "vm/heap.h"

#include "util/hash.h"

/* Globals */
List EmptyList = {
  .tag        = T_LIST,
  .nosweep    = true,
  .gray       = true,
  .sealed     = true,
  .head       = NUL,
  .cnt        = 0,
  .tail       = &EmptyList,
};

// VTables
void   trace_list(State* vm, void* x);
size64 pr_list(State* vm, Port* p, Val x);
hash64 hash_list(Val x);
bool   egal_lists(Val x, Val y);
int    order_lists(Val x, Val y);
bool   list_empty(void* x);
Val    list_first(void* x);
void*  list_rest(void* x);


VTable ListVt = {
  .code    = T_LIST,
  .name    = "List",
  .obsize  = sizeof(List),
  .is_seq  = true,
  .tracefn = trace_list,
  .prfn    = pr_list,
  .hashfn  = hash_list,
  .egalfn  = egal_lists,
  .orderfn = order_lists,
  .emptyfn = list_empty,
  .firstfn = list_first,
  .restfn  = list_rest,
};

void   trace_pair(State* vm, void* x);
size64 pr_pair(State* vm, Port* p, Val x);

VTable PairVt = {
  .code    = T_PAIR,
  .name    = "Pair",
  .obsize  = sizeof(Pair),
  .tracefn = trace_pair,
  .prfn    = pr_pair,
};

/* Internal APIs */
void init_list(List* l, Val h, size64 c, List* t) {
  l->head = h;
  l->cnt  = c;
  l->tail = t;
}

/* Runtime APIs & Interfaces */
// list APIs
// lifetime
void trace_list(State* vm, void* x) {
  List* l = x;

  mark(vm, l->head);
  mark(vm, l->tail);
}

// comparison
hash64 hash_list(Val x) {
  List* lx = as_list(x);

  // should never be called on an empty list
  assert(lx->cnt != 0);

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

int order_lists(Val x, Val y) {
  List* xs = as_list(x), * ys = as_list(y);

  int out = 0;

  for ( ; out == 0 && xs->cnt && ys->cnt; xs=xs->tail, ys=ys->tail )
    out = rl_order(xs->head, ys->head);

  return out ? : 0 - (xs->cnt == 0) + (ys->cnt == 0);
}

// sequence
bool list_empty(void* x) {
  List* xs = x;

  return xs->cnt == 0;
}

Val list_first(void* x) {
  List* xs = x;

  return xs->head;
}

void* list_rest(void* x) {
  List* xs = x;

  return xs->cnt == 1 ? NULL : xs->tail;
}

// print
size64 pr_list(State* vm, Port* p, Val x) {
  (void)vm;

  List* xs = as_list(x);
  size64 o = rl_putc(p, '(');

  for ( ; xs->cnt; xs=xs->tail ) {
    o += rl_pr(p, xs->head);

    if ( xs->cnt > 1 )
      o += rl_putc(p, ' ');
  }

  o += rl_putc(p, ')');

  return o;
}

// Pair APIs
void trace_pair(State* vm, void* x) {
  Pair* p = x;

  mark(vm, p->car);
  mark(vm, p->cdr);
}

size64 pr_pair(State* vm, Port* p, Val x) {
  (void)vm;

  Pair* c   = as_pair(x);
  size64 r  = rl_putc(p, '(');
  r        += rl_pr(p, c->car);
  r        += rl_printf(p, " . ");
  r        += rl_pr(p, c->cdr);
  r        += rl_putc(p, ')');

  return r;
}

/* External APIs */
// List API
List* mk_list2(Val hd, List* tl) {
  List* l = new_obj(&Vm, T_LIST, MF_SEALED);

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

      init_obj(&Vm, (Obj*)spc, T_LIST, MF_SEALED);
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

// Pair API
Pair* mk_pair(Val a, Val d) {
  Pair* r = new_obj(&Vm, T_PAIR, MF_NOHASH);
  r->car  = a;
  r->cdr  = d;

  return r;
}

// initialization
void rl_toplevel_init_list(void) {
  // compute empty list hash
  EmptyList.hash = mix_hashes(hash_word(T_LIST), hash_pointer(&EmptyList));
}
