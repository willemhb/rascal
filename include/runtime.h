#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "rascal.h"
#include "describe/utils.h"

// globals --------------------------------------------------------------------
extern value_t r_main, r_argc, r_args, r_kw_ok;

// memory management ----------------------------------------------------------
size_t  calc_array_size( size_t n_bytes );

bool    check_heap_overflow( size_t n_bytes );
bool    check_stack_overflow( size_t n_vals );
bool    in_heap( void *p );
bool    in_stack( void *p );
bool    is_managed( value_t x );

void   *allocate( size_t n_bytes );
void   *allocate_array( size_t n, size_t size );
void   *allocate_table( size_t base, size_t n, size_t size );

#define allocate_words(n)  allocate_array(n, sizeof(value_t))
#define allocate_pairs(n)  allocate_array(n, sizeof(cons_t))
#define allocate_vector(n) allocate_table(sizeof(vector_t),n,sizeof(value_t))

int     collect_garbage( void );
value_t relocate( value_t x );
value_t forward( value_t x );
void    trace( value_t x );

// stack manipulation ---------------------------------------------------------
index_t push( value_t x );
index_t pushn( size_t n );
index_t push_s( const char *fname, value_t x );
index_t pushn_s(const char *fname, value_t x );

value_t pop( void );
value_t popn( size_t n );
value_t pop_s( const char *fname );
value_t popn_s( const char *fname, size_t n );

// error handling -------------------------------------------------------------
void    error(const char *fname, const char *fmt, ...);
void    require(const char *fname, bool test, const char *fmt, ...);
size_t  argc( const char *fname, size_t got, size_t expect );
size_t  vargc( const char *fname, size_t got, size_t expect );
size_t  oargc( const char *fname, size_t got, size_t n, ...);
type_t  argt( const char *fname, value_t got, type_t expect );
type_t  oargt( const char *fname, value_t got, size_t n, ...);
size_t  s_argc( const char *fname, value_t form, size_t expect );
size_t  s_vargc( const char *fname, value_t form, size_t expect );

// builtins -------------------------------------------------------------------
void r_builtin(exit);
void r_builtin(getenv);
void r_builtin(system);
void r_builtin(errorb);

// initialization -------------------------------------------------------------
void runtime_init( void );

#endif
