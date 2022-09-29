
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

// utilities
val_t ncat(val_t xs, val_t ys)
{
  if (xs == NIL)
    return ys;

  if (ys == NIL)
    return xs;

  arity_t inc = as_cons(ys)->len;

  for (val_t tmp=xs;;tmp=as_cons(xs)->tl)
    {
      as_cons(tmp)->len += inc;

      if (!is_cons(as_cons(tmp)->tl))
	{
	  as_cons(tmp)->tl = ys;
	  break;
	}
    }
  return xs;
}

val_t xar( val_t xs, val_t v )
{
  if (!!(as_obj(xs)->flags & 0x1e))
    {
      cons_t *copy = new_cons(v, as_cons(xs)->tl);
      xs = tag_val((obj_t*)copy, OBJECT);
    }

  else
    as_cons(xs)->hd = v;

  return xs;
}

val_t xdr( val_t xs, val_t v )
{
  if (is_proper(xs))
    {
      cons_t *copy = new_cons(as_cons(xs)->hd, v);
      xs = tag_val((obj_t*)copy, OBJECT);
    }

  else
    {
      as_cons(xs)->tl = v;
      as_cons(xs)->len = 1;

      if (is_cons(v))
	as_cons(xs)->len += as_cons(v)->len;

    }
  return xs;
}

val_t nrev(val_t xs)
{
  if (is_cons(xs) && as_cons(xs)->len > 1)
    {
      val_t last=NIL;
      arity_t len = as_cons(xs)->len;

      for (arity_t i=1; i <= len; i++)
      	{
	  as_cons(xs)->len = i;
	  val_t tmp        = as_cons(xs)->tl;
	  as_cons(xs)->tl  = last;
	  last             = xs;
	  xs               = tmp;
	}
    }

  return xs;
}
