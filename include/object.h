#ifndef rascal_object_h
#define rascal_object_h

// general predicates, dispatch, utilities, and object model
#include "rascal.h"

bool numberp( value_t x );
bool nilp( value_t x );
bool truep( value_t x );
bool falsep( value_t x );

bool fixnump( value_t x );
bool symp( value_t x );
bool consp( value_t x );
bool vecp( value_t x );
bool strp( value_t x );
bool funp( value_t x );

bool listp( value_t x );
bool builtinp( value_t x );
bool arrayp( value_t x );
bool closurep( value_t x );
bool gensymp( value_t x );

bool forwardp( value_t x );
bool inlinep( value_t x );

size_t r_size( value_t x );

uint *_alen( value_t x );
void *_adata( value_t x );

#define alen( x )  (_alen( x )[0])
#define adata( x ) _adata( x )
#define sdata( x ) ((char*)adata( x ))
#define vdata( x ) ((value_t*)adata( x ))
#define asize( x ) (((uint*)adata( x ))[-1])

symbol_t *mk_symbol( char *n, int l, hash_t h, bool i );
cons_t *mk_cons( void );
value_t mk_bool( bool x );
value_t mk_num( double x );
value_t mk_fixnum( long x );

value_t symbol( char *name );

#define car( x ) (((cons_t*)ptrval(x))->car)
#define cdr( x ) (((cons_t*)ptrval(x))->cdr)

#define sbind( x )  (((symbol_t*)ptrval(x))->bind)
#define shash( x )  (((symbol_t*)ptrval(x))->hash)
#define sidno( x )  (((symbol_t*)ptrval(x))->idno)
#define sname( x )  (&(((symbol_t*)ptrval(x))->name[0]))
#define slen( x )   (((symbol_t*)ptrval(x))->len)
#define sentry( x ) ((symbols_t*)(ptrval(x)-sizeof(cons_t)))

#define vhead( x ) ((x)[-1])
#define shead( x ) (((value_t*)(x))[-1])

#define clform( x ) ( vdata( x )[0] )
#define clname( x ) ( vdata( x )[1] )
#define clenvt( x ) ( vdata( x )[2] )
#define clvals( x ) ( vdata( x )[3] )
#define clcode( x ) ( vdata( x )[4] )

#define envarg( x ) ( vdata( x )[0] )
#define envloc( x ) ( vdata( x )[1] )
#define envclo( x ) ( vdata( x )[2] )
#define envupv( x ) ( vdata( x ) + 3 )


#endif
