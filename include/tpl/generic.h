#ifndef rl_tpl_generic_h
#define rl_tpl_generic_h

#include "tpl/type.h"

#define gen_decl( r, m )                        \
  r obj_##m( obj_t *obj );                      \
  r val_##m( val_t val )

#define gen_call( x, m )                                 \
  _Generic((x),                                          \
           val_t:val_##m,                                \
           obj_t*:obj_##m)((x))

#endif
