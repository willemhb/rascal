#ifndef rl_data_tuple_h
#define rl_data_tuple_h

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
#include "data/base.h"

// macros ---------------------------------------------------------------------
#define is_tuple(x)    has_type(t, EXP_TUPLE)
#define as_tuple(x)    ((Tuple*)as_obj(x))
#define tuple_arity(t) ((t)->flags)

// C types --------------------------------------------------------------------
struct Tuple {
  HEAD;

  Expr slots[0];
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Tuple* mk_tuple(size_t n, Expr* xs);
Expr   tuple_ref(Tuple* t, size_t o);

// initialization -------------------------------------------------------------
void toplevel_init_data_tuple(void);

#endif
