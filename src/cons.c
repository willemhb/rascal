// standard headers -----------------------------------------------------------
// rascal headers -------------------------------------------------------------
#include "cons.h"
#include "memory.h"

// utilities ------------------------------------------------------------------
void init_pair(pair_t *xp, value_t ca, value_t cd)
{
  uint arity = ispair(cd) ? ob_arity(cd) : 1;
  init_object(asob(xp), arity, type_pair, sizeof(pair_t) );

  car(xp) = ca;
  cdr(xp) = cd;
}

void init_cons(cons_t *xc, value_t ca, value_t cd)
{
  uint arity = ispair(cd) ? ob_arity(cd) : 1;
  init_object(asob(xc), arity, type_pair, sizeof(pair_t) );

  car(xc) = ca;
  cdr(xc) = cd;  
}

void init_entry(entry_t *xp, value_t key, value_t bind, ulong hash, uint order)
{
  init_object(asob(xp), order, type_entry, sizeof(entry_t) );

  car(xp)   = key;
  cdr(xp)   = bind;
  ehash(xp) = hash;
}

value_t pair(value_t ca, value_t cd)
{
  preserve( 2, &ca, &cd );

  pair_t *out = allocate(type_pair, 1, 0 );
  init_pair(out, ca, cd);
  return tagp(out);
}

value_t cons(value_t ca, value_t cd)
{
  preserve( 2, &ca, &cd );
  cons_t *out = allocate(type_cons, 1, 0);
  init_cons(out, ca, cd);
  return tagp(out);
}

// initialization -------------------------------------------------------------
