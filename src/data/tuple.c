/**
 *
 * Minimal aggregate data type. Basic workhorse data structure.
 *
 * For maximum compactness, Tuples store their arity in the `flags`
 * field of the object header and store their slots immediately, rather
 * than in an owned pointer.
 * 
 * When custom aggregate types are implemented they'll probably be
 * based on tuples.
 *
 **/
// headers --------------------------------------------------------------------
#include <string.h>

#include "data/tuple.h"

#include "lang/base.h"

#include "sys/error.h"
#include "sys/memory.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------


// function prototypes --------------------------------------------------------
void   print_tuple(Port* ios, Expr x);
bool   egal_tuples(Expr x, Expr y);
void*  alloc_tuple(ExpType type, flags_t flags, size_t n);
size_t tuple_size(void* ptr);
void   trace_tuple(void* ptr);
Tuple* new_tuple(size_t n);

// globals --------------------------------------------------------------------
ExpAPI TupleExpAPI = {
  .print_fn = print_tuple,
  .egal_fn  = egal_tuples,
};

ObjAPI TupleObjAPI = {
  .obsize   = sizeof(Tuple),
  .size_fn  = tuple_size,
  .alloc_fn = alloc_tuple,
  .trace_fn = trace_tuple,
};

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_tuple(Port* ios, Expr x) {
  Tuple* t = as_tuple(x); size_t tn = tuple_arity(t);

  pprintf(ios, "(");

  print_exp_array(ios, tn, t->slots, ", ", false);

  pprintf(ios, ")");
}

bool egal_tuples(Expr x, Expr y) {
  Tuple* tx = as_tuple(x), * ty = as_tuple(y);
  size_t xn = tuple_arity(tx), yn = tuple_arity(ty);

  return egal_exp_arrays(xn, tx->slots, yn, ty->slots);
}

void* alloc_tuple(ExpType type, flags_t flags, size_t n) {
  (void)type;
  (void)flags;

  size_t asize = sizeof(Tuple) + n * sizeof(Expr);
  void* out    = allocate(true, asize);

  return out;
}

size_t tuple_size(void* ptr) {
  Tuple* t = ptr;

  return sizeof(Tuple) + tuple_arity(t) * sizeof(Expr);
}

void trace_tuple(void* ptr) {
  Tuple* t = ptr;

  trace_exp_array(tuple_arity(t), t->slots);
}

Tuple* new_tuple(size_t n) {
  Tuple* out = mk_obj(EXP_TUPLE, 0, n);
  
  tuple_arity(out) = n;

  return out;
}

// external -------------------------------------------------------------------
Tuple* mk_tuple(size_t n, Expr* xs) {
  assert(xs != NULL);

  Tuple* out = new_tuple(n);

  memcpy(out->slots, xs, n*sizeof(Expr));

  return out;
}

Expr tuple_ref(Tuple* t, size_t n) {
  require(n < tuple_arity(t), "tuple index %zu out of bounds", n);

  return t->slots[n];
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_tuple(void) {
     Types[EXP_TUPLE] = (TypeInfo) {
       .type    = EXP_TUPLE,
       .c_name  = "tuple",
       .exp_api = &TupleExpAPI,
       .obj_api = &TupleObjAPI
     };
}
