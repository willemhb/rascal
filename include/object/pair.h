#ifndef rascal_pair_h
#define rascal_pair_h

#include "rascal.h"
#include "describe/safety.h"

typedef enum {
  pairfl_proper  =0x10,
  pairfl_assc    =0x20,
  pairfl_mutable =0x40
} pairflags_t;

struct pair_t {
  object_t base;

  union {
    size_t len;
    hash_t hash;
  };

  value_t car, cdr;
};

#define aspair(x)  ((pair_t*)ptr(x))

#define car(x)     ( aspair(x)->car )
#define cdr(x)     ( aspair(x)->cdr )
#define plen(x)    ( aspair(x)->len )
#define phash(x)   ( aspair(x)->hash )

// api ------------------------------------------------------------------------
bool_t  pairp( value_t x ); // a common pair
bool_t  asscp( value_t x ); // a pair storing a key, hash, and binding
bool_t  listp( value_t x );


value_t   rsc_pair( value_t ca, value_t cd );
value_t   rsc_assc( value_t key, value_t bind, ulong_t hash );
value_t   rsc_cons(value_t ca, value_t cd );
value_t   rsc_list( value_t *base, size_t n );

// accessors ------------------------------------------------------------------
value_t pair_car( value_t xp );
value_t pair_cdr( value_t xp );
value_t pair_xar( value_t xp, value_t xc );
value_t pair_xdr( value_t xp, value_t xc );

// initialization -------------------------------------------------------------
void    pair_init( void );

#endif
