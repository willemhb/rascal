#ifndef rascal_object_h
#define rascal_object_h

// general predicates, dispatch, utilities, and object model
#include "rascal.h"

bool nilp( value_t x );
bool truep( value_t x );
bool falsep( value_t x );

bool fixnump( value_t x );
bool symbolp( value_t x );
bool vectorp( value_t x );
bool consp( value_t x );
bool closurep( value_t x );
bool builtinp( value_t x );
bool stringp( value_t x );

bool listp( value_t x );
bool forwardp( value_t x );
bool inlinep( value_t x );
bool boxedp( value_t x );
bool gensymp( value_t x );
bool keywordp( value_t x );
bool boundp( value_t x );

cons_t     *tocons( char *fname, value_t x );
symbol_t   *tosymbol( char *fname, value_t x );
vector_t   *tovector( char *fname, value_t x );
string_t   *tostring( char *fname, value_t x );
bytecode_t *tobytecode( char *fname, value_t x );
closure_t  *toclosure( char *fname, value_t x );
builtin_t   tobuiltin( char *fname, value_t x );
boolean_t   tobool( char *fname, value_t x );
fixnum_t    tofixnum( char *fname, value_t x );

size_t r_size( value_t x );
type_t r_type( value_t x );

#define asob( x )       ((object_t*)pval(x))
#define asbigob( x )    ((big_object_t*)(pval(x)-sizeof(cons_t)))
#define ascons( x )     ((cons_t*)pval(x))
#define asvector( x )   ((vector_t*)asbigob( x ))
#define asstring( x )   ((string_t*)asbigob( x ))
#define asbytecode( x ) ((bytecode_t*)asbigob( x ))
#define assymbol( x )   ((symbol_t*)pval(x))
#define asclosure( x )  ((closure_t*)pval(x))
#define asbuiltin( x )  ((builtin_t)pval(x))

#define inlined( x )  (asob( x )->inlined)
#define Ctype( x )    (asob( x )->Ctype)
#define encoding( x ) (asob( x )->encoding)
#define boxed( x )    (asob( x )->boxed)
#define type( x )     (asob( x )->type)
#define flags( x )    (asob( x )->flags)
#define offset( x )   (asob( x )->flags)

#define length( x ) (asbigob( x )->length)

#define adata( x ) _odata( x )
#define sdata( x ) ((char*)adata( x ))
#define instr( x ) ((short*)adata( x ))
#define vdata( x ) ((value_t*)adata( x ))

symbol_t  *mk_symbol( char *n, size_t l, hash_t h, bool i );
cons_t    *mk_cons( void );
vector_t  *mk_vector( size_t n );
closure_t *mk_closure( void );
string_t  *mk_string( size_t n );
value_t    mk_bool( int x );
value_t    mk_fixnum( long x );

void *_odata( value_t x );

typedef enum {
  symfl_const = 0x10,
  symfl_bound = 0x20
} symbol_fl_t;

value_t symbol( char *name );
value_t gensym( char *fname );

value_t *cons_s( value_t *ca, value_t *cd );
value_t *vector_s( size_t n, value_t *a );
value_t *string_s( size_t n, value_t *a );
value_t *bytecode_s( size_t n, value_t *a );

// these procedures are all safe
value_t *aref_s( char *fname, value_t *x,  value_t *n );
value_t *axef_s( char *fname, value_t *x, value_t *n, value_t *v );
value_t *aput_s( char *fname, value_t *x, value_t *v );

value_t ncat( value_t x, value_t y );
value_t nrev( value_t x );
value_t assoc( value_t k, value_t xs );
size_t  clength( value_t x );

#define car( x )   get( cons, x, car )
#define cdr( x )   get( cons, x, cdr )

#define car_s( f, x ) get_s( f, cons, x, car )
#define cdr_s( f, x ) get_s( f, cons, x, cdr )

#define cddr( x )  cdr( cdr( x ) )
#define cdar( x )  cdr( car( x ) )
#define cadr( x )  car( cdr( x ) )
#define caar( x )  car( car( x ) )

#define sbind( x )        get( symbol, x, bind )
#define shash( x )        get( symbol, x, hash )
#define sidno( x )        get( symbol, x, idno )
#define sflags( x )       get( symbol, x, flags )
#define sname( x )        get( symbol, x, name )
#define sbind_s( f, x )   get_s( f, symbol, x, bind )
#define shash_s( f, x )   get_s( f, symbol, x, hash )
#define sidno_s( f, x )   get_s( f, symbol, x, idno )
#define sflags_s( f, x )  get_s( f, symbol, x, flags )
#define sname_s( f, x )   get_s( f, symbol, x, name )

#define sentry( x ) ((symbols_t*)(pval(x)-sizeof(cons_t))) 

#endif
