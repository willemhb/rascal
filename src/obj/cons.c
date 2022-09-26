#include <stdio.h>

#include "obj/cons.h"
#include "obj/str.h"
#include "obj/table.h"
#include "mem.h"

// api
void    init_cons( cons_t *new, val_t hd, val_t tl, type_t eltype )
{
  init_obj((obj_t*)new,
	   cons_type,
	   (tl == NIL || is_proper(tl))*cons_fl_proper );

  new->hd     = hd;
  new->tl     = tl;
  new->eltype = eltype;

  if (is_cons(tl))
    new->len = as_cons(tl)->len + 1;

  else
    new->len = 1;
}

void init_kv( kv_t *new, val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  init_cons((cons_t*)new, k, b, t );
  new->cons.len = i;
  new->hash = h;
}

void init_var( var_t *new, val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  init_kv( (kv_t*)new, k, b, t, i, h );
  new->doc  = empty_str;
  new->meta = (obj_t*)empty_table;
}

cons_t *new_cons( val_t hd, val_t tl )
{
  cons_t *out = alloc( sizeof(cons_t) );
  init_cons( out, hd, tl, any_type );
  return out;
}

kv_t *new_kv( val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  kv_t *out = alloc(sizeof(kv_t));
  init_kv( out, k, b, t, i, h );
  return out;
}

var_t *new_var( val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  var_t *out = alloc(sizeof(var_t));
  init_var( out, k, b, t, i, h );
  return out;
}
