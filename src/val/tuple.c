#include <string.h>

#include "val/tuple.h"
#include "val/port.h"
#include "util/util.h"
#include "vm.h"
#include "lang.h"

// C types --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
void print_tuple(Port* ios, Expr x);
hash_t hash_tuple(Expr x);
bool egal_tuples(Expr x, Expr y);
void trace_tuple(RlState* rls, void* ptr);
void free_tuple(RlState* rls, void* ptr);

// Globals --------------------------------------------------------------------
// Type object
Type TupleType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_TUPLE,
  .obsize   = sizeof(Tuple),
  .print_fn = print_tuple,
  .hash_fn  = hash_tuple,
  .egal_fn  = egal_tuples,
  .trace_fn = trace_tuple,
  .free_fn  = free_tuple,
};

// Implementations ------------------------------------------------------------
Tuple* mk_tuple(RlState* rls, int argc) {
  Tuple* out = mk_obj(rls, &TupleType, 0);
  out->count = argc;

  if ( argc == 0 ) {
    out->data = NULL;
  } else {
    out->data = allocate(NULL, argc*sizeof(Expr));
    memcpy(out->data, rls->s_top-argc, argc*sizeof(Expr));
  }

  return out;
}

Tuple* mk_tuple_s(RlState* rls, int argc) {
  Tuple* out = mk_tuple(rls, argc);
  stack_push(rls, tag_obj(out));

  return out;
}

void print_tuple(Port* ios, Expr x) {
  pprintf(ios, "[");
  Tuple* t = as_tuple(x);

  for ( int i=0; i<t->count; i++ ) {
    print_expr(ios, t->data[i]);

    if ( i+1 < t->count )
      pprintf(ios, " ");
  }

  pprintf(ios, "]");
}

hash_t hash_tuple(Expr x) {
  Tuple* tx = as_tuple(x);
  hash_t hash = hash_word_48(tx->count+1); // seed value

  for ( int i=0; i<tx->count; i++ ) {
    hash_t xh = hash_expr(tx->data[i]);
    hash = mix_hashes_48(hash, xh);
  }

  return mix_hashes_48(TupleType.hashcode, hash);
}

bool egal_tuples(Expr x, Expr y) {
  Tuple* tx = as_tuple(x), *ty = as_tuple(y);

  bool out = tx->count != ty->count;

  for (int i=0; out && i<tx->count; i++ )
    out = egal_exprs(tx->data[i], ty->data[i]);

  return out;
}

void trace_tuple(RlState* rls, void* ptr) {
  Tuple* t = ptr;

  trace_expr_array(rls, t->data, t->count);
}

void free_tuple(RlState* rls, void* ptr) {
  (void)rls;
  Tuple* t = ptr;

  release(NULL, t->data, 0);
}
