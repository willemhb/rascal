#include <string.h>

#include "memutils.h"
#include "hashing.h"

#include "object.h"
#include "number.h"
#include "list.h"
#include "array.h"
#include "function.h"
#include "runtime.h"

// utilities ------------------------------------------------------------------
bool Cbool( value_t x ) { return x != val_false && x != val_nil; }

// predicates -----------------------------------------------------------------
mk_tag_p(immediate)
mk_tag_p(boxed)
mk_tag_p(pair)

bool is_empty( value_t x ) {
  return is_immediate(x) &&
    (is_list(x)   ||
     is_vector(x) ||
     is_binary(x) ||
     is_string(x) );
}


// core -----------------------------------------------------------------------
type_t r_type( value_t x ) {
  switch (tag(x)) {
  case tag_immediate: return x>>24;
  case tag_boxed: return ob_type(x) >> 3;
  default: return tag(x);
  }
}

size_t r_size( value_t x ) {
  if (tag(x) == tag_fixnum)
    return 8;

  if (tag(x) == tag_immediate)
    return is_empty( x ) ? 0 : 4;

  if (sizeof_dispatch[r_type(x)])
    return sizeof_dispatch[r_type(x)](x);

  return ob_base_size(x);
}

int r_order( value_t x, value_t y ) {
  if (x == y)
    return 0;

  int xtag = tag(x), ytag = tag(y), o;
  type_t xtype = r_type( x ), ytype = r_type( y );

  /* if different types, order arbitrarily by integer value of type */
  if ((o=ord_uint(xtype, ytype)))
    return o;

  /* order empty objects before non-empty without having to check for unsafe access later */
  if ((o=ord_uint(xtag, ytag)))
    return o;

  if (xtag == tag_fixnum)
    return ord_long( ival(x), ival(y) );

  if (xtag == tag_immediate)
    return ord_int( cval(x), cval(y) );

  if (order_dispatch[xtype])
    return order_dispatch[xtype](x, y);

  return ord_ulong( x&~(value_t)7, y&~(value_t)7 );
}

// builtins ------------------------------------------------------------------
void r_builtin(is_id) {
  argc( "id?", n, 2 );

  Stack[Sp-2] = boolean( Stack[Sp-2] == Stack[Sp-1] );
  Sp--;
}

void r_builtin(is_eql) {
  argc( "=?", n, 2 );

  Stack[Sp-2] = boolean( r_order(Stack[Sp-2], Stack[Sp-1]) == 0 );
  Sp--;
}

void r_builtin(size) {
  argc( "sizeof", n, 1 );
  Tos = fixnum( r_size(Tos) );
}

void r_builtin(ord) {
  argc( "ord", n, 2);

  int o = r_order(Stack[Sp-2], Stack[Sp-1]);
  Stack[(Sp--)-2] = fixnum(o);
}

void r_builtin(not) {
  argc( "not", n, 1 );
  Tos = Cbool( Tos ) ? val_false : val_true;
}

// initialization -------------------------------------------------------------
void object_init( void ) {
  // create builtins
  builtin( "sizeof", builtin_size );
  builtin( "ord", builtin_order );
  builtin( "id?", builtin_is_id );
  builtin( "=?", builtin_is_eq );
  builtin( "not", builtin_not );
}
