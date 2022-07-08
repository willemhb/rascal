#ifndef rascal_interface_list_h
#define rascal_interface_list_h

#include "rascal.h"

#define PairType(_keytype, _key, _car, _cdr)	\
  {						\
  object_t base;				\
  _keytype _key;				\
  value_t _car, _cdr;				\
  }

enum listfl_t {
  listfl_proper = 0x10,
  listfl_sexpr  = 0x20,
};

struct pair_t PairType(size_t, len, car, cdr);
struct assc_t PairType(hash_t, hash, key, bind);
struct envt_t PairType(size_t, len, names, binds);

#define aspair(x)  ((pair_t*)ptr(x))
#define asassc(x)  ((assc_t*)ptr(x))
#define asenvt(x)  ((envt_t*)ptr(x))

#define car(x)     ( aspair(x)->car )
#define cdr(x)     ( aspair(x)->cdr )
#define plen(x)    ( aspair(x)->len )

#define akey(x)    ( asassc(x)->key )
#define ahash(x)   ( asassc(x)->hash )
#define abind(x)   ( asassc(x)->bind )

#define elen(x)    ( asenvt(x)->len )
#define enames(x)  ( asenvt(x)->names )
#define ebinds(x)  ( asenvt(x)->binds )

// api ------------------------------------------------------------------------
bool_t    listp( value_t x );
bool_t    properp( value_t x );
bool_t    sexprp( value_t x );

object_t *list_construct( type_t t, flags_t f, size_t n );
int_t     list_init( type_t t, flags_t f, size_t n, value_t i, void *spc );
value_t   list_relocate( value_t x );
size_t    list_print( FILE *ios, value_t x );
int_t     list_order( value_t x, value_t y );

// utilities ------------------------------------------------------------------
size_t    list_len( value_t x );
value_t   pair_xar( value_t x, value_t a );
value_t   pair_xdr( value_t x, value_t a );

// initialization -------------------------------------------------------------
void      init_list( void );

#endif
