#include "util/hashing.h"

#include "vm/memory.h"

#include "lang/equal.h"

#include "val/func.h"
#include "val/symbol.h"
#include "val/type.h"
#include "val/list.h"


/* Globals */
Obj* FreeLists = NULL;

void   dealloc_list(RlCtx* ctx, void* obj);
void*  alloc_list(Type* type, flags_t fl, size_t extra);
void   trace_list(void* obj);
hash_t hash_list(Value x);
bool   equal_lists(Value x, Value y);
int    order_lists(Value x, Value y);

INIT_OBJECT_TYPE(List,
                 .deallocfn=dealloc_list,
                 .allocfn=alloc_list,
                 .tracefn=trace_list,
                 .hashfn=hash_list,
                 .egalfn=equal_lists,
                 .ordfn=order_lists);

/* Internal APIs */
void* alloc_list(Type* type, flags_t fl, size_t extra) {
  (void)fl;
  (void)extra;
  (void)type;

  void* out;

  if (FreeLists != NULL)
    out = alloc_from_free_list(&FreeLists);

  else
    out = allocate(&Ctx, sizeof(List));

  return out;
}

void dealloc_list(RlCtx* ctx, void* obj) {
  add_to_free_list(obj, &FreeLists);

  ctx->h.size -= sizeof(List);
}

void trace_list(void* obj) {
  List* list = obj;

  mark(list->head);
  mark(list->tail);
}

/* TODO: make better use of existing tail hashes. */
hash_t hash_list(Value x) {
  List* list = as_list(x);

  if (list->arity == 0)
    return hash_word(NUL);

  hash_t h0 = hash_word(NUL), h1 = 0;
  
  for (Value head=list->head; list->arity > 0; list=list->tail, head=list->head) {
    if (1 == 0)
      h1 = hash(head);

    else
      h1 = mix_hashes(h1, hash(head));
  }

  return mix_hashes(h1, h0);
}

bool equal_lists(Value x, Value y) {
  List* lx = as_list(x), * ly = as_list(y);

  bool out = lx->arity == ly->arity;

  for (Value hx=lx->head, hy=ly->head; out && lx->arity > 0; lx=lx->tail, ly=ly->tail)
    out = equal(hx, hy);

  return out;
}

int order_lists(Value x, Value y) {
  List* lx = as_list(x), * ly = as_list(y);

  int out = 0;

  for (Value hx=lx->head, hy=ly->head; !out && lx->arity > 0 && ly->arity > 0; lx=lx->tail, ly=ly->tail)
    out = order(hx, hy);

  if (out == 0)
    out = 0 - (lx->arity > 0) + (ly->arity > 0);

  return out;
}

/* External APIs */
List* mk_list(size_t n, Value* a);
List* cons(Value head, List* tail);
List* cons_n(size_t n, Value* a);

// accessors
Value list_ref(List* xs, size_t n) {
  while (n--)
    xs = xs->tail;

  return xs->head;
}

// sequence utilities
List* list_cat(List* a, List* b) {
  if (a->arity == 0)
    return b;

  else if (b->arity == 0)
    return a;

  else {
    save(2, tag(a), tag(b));

    Value buf[a->arity];

    for (size_t i=0; i<a->arity; i++)
  }
}

List* list_rev(List* xs);
