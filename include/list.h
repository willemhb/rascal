#ifndef rascal_list_h
#define rascal_list_h

#include "rascal.h"

// constructors --------------------------------------------------------------
value_t cons( value_t car, value_t cdr );
value_t list( size_t n, value_t *args );
value_t consn( size_t n, value_t *args );
value_t list1( value_t x);
value_t list2( value_t x, value_t y );
index_t list_s( size_t n, value_t *args );
index_t consn_s( size_t n, value_t *args );
index_t cons_s( value_t *car, value_t *cdr );

// accessors & utilities ------------------------------------------------------
value_t cons_nth( value_t ca, size_t n );
value_t cons_xth( value_t c, size_t n, value_t x );
value_t cons_nth_s( const char *fname, value_t c, long n );
value_t cons_xth_s( const char *fname, value_t c, long n, value_t x );
size_t  list_length( value_t x );

// predicates -----------------------------------------------------------------
bool is_cons( value_t x );
bool is_nil( value_t x );
bool is_list( value_t x );

// methods --------------------------------------------------------------------
size_t list_prin( FILE *ios, value_t c );
int    list_order( value_t x, value_t y );
size_t list_sizeof( value_t x );

// safecasts ------------------------------------------------------------------
cons_t *tocons( const char *fname, value_t x );

// methods --------------------------------------------------------------------
int    list_order( value_t x, value_t y );
size_t list_prin( FILE *ios, value_t x );
size_t list_size( value_t x );

// builtins -------------------------------------------------------------------
void builtin_cons(size_t n);
void builtin_consn(size_t n);
void builtin_list(size_t n);
void builtin_car(size_t n);
void builtin_cdr(size_t n);
void builtin_xar(size_t n);
void builtin_xdr(size_t n);
void builtin_is_cons(size_t n);
void builtin_is_nil(size_t n);
void builtin_is_list(size_t n);

// initialization -------------------------------------------------------------
void list_init( void );

// utility macros -------------------------------------------------------------
#define ascons(x) ((cons_t*)pval(x))
#define car(x)    (ascons(x)->car)
#define cdr(x)    (ascons(x)->cdr)

#define cddr(x)   car(cdr(x))
#define cdar(x)   cdr(car(x))
#define cadr(x)   car(cdr(x))
#define caar(x)   car(car(x))

#define caadr(x)  car(car(cdr(x)))

#define car_s(f, x) (tocons(f,x)->car)
#define cdr_s(f, x) (tocons(f,x)->cdr)


#endif
