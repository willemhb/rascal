#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "rascal.h"
#include "describe/utils.h"

// memory management ----------------------------------------------------------
size_t calc_array_size( size_t n_bytes_requested );
void *allocate( size_t n_bytes );
int collect_garbage( void );

// constructors ---------------------------------------------------------------
value_t cons( value_t ca, value_t cd );
index_t cons_s( value_t *ca, value_t *cd );
value_t symbol( char *name );
value_t fixnum( long x );

// safecasts ------------------------------------------------------------------
cons_t   *tocons( const char *fname, value_t x );
symbol_t *tosymbol( const char *fname, value_t x );
fixnum_t  tofixnum( const char *fname, value_t x );

// stack manipulation ---------------------------------------------------------
index_t push( value_t x );
index_t pushn( size_t n );

value_t pop( void );
value_t popn( size_t n );

// error handlign -------------------------------------------------------------
void    error(const char *fname, const char *fmt, ...);
void    require(const char *fname, bool test, const char *fmt, ...);

// convenience macros ---------------------------------------------------------
#define car(x) get( cons, x, car )
#define cdr(x) get( cons, x, cdr )

#define car_s(f, x) get_s( f, cons, x, car )
#define cdr_s(f, x) get_s( f, cons, x, cdr )

#define sbind(x) get( symbol, x, bind )
#define sname(x) get( symbol, x, name )
#define shash(x) get( symbol, x, hash )
#define sidno(x) get( symbol, x, idno )

#define sbind_s(f, x) get_s( f, symbol, x, bind )
#define sname_s(f, x) get_s( f, symbol, x, name )
#define shash_s(f, x) get_s( f, symbol, x, hash )
#define sidno_s(f, x) get_s( f, symbol, x, idno )

#endif
