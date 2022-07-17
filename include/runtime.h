#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "rascal.h"
#include "describe/utils.h"

// memory management ----------------------------------------------------------
size_t  calc_array_size( size_t n_bytes );

bool    check_heap_overflow( size_t n_bytes );
bool    check_stack_overflow( size_t n_vals );
bool    is_managed( value_t x );

void   *allocate( size_t n_bytes );
value_t reallocate( value_t x, size_t n );
int     collect_garbage( void );
value_t relocate( value_t x );
value_t forward( value_t x );
void    trace( value_t x );

pair_t   *mk_pair( void );
node_t   *mk_node( void );
cons_t   *mk_cons( size_t n );
table_t  *mk_table( size_t n );
vector_t *mk_vector( size_t n );
string_t *mk_string( size_t n );
code_t   *mk_code( size_t n );

// utilities ------------------------------------------------------------------
type_t r_type( value_t x );

bool consp( value_t x );
bool symbolp( value_t x );
bool fixnump( value_t x );
bool functionp( value_t x );
bool builtinp( value_t x );
bool closurep( value_t x );
bool arrayp( value_t x );
bool vectorp( value_t x );
bool stringp( value_t x );
bool tablep( value_t x );
bool codep( value_t x );
bool portp( value_t x );
bool nilp( value_t x );
bool keywordp( value_t x );

hash_t r_hash( value_t x );
int    r_order( value_t x, value_t y );

// constructors ---------------------------------------------------------------
value_t cons( value_t ca, value_t cd );
index_t cons_s( value_t *ca, value_t *cd );

value_t vector( size_t n, ...);
index_t vector_s( size_t n, value_t *vals );

value_t string( char *chrs );
index_t string_s( size_t n, char *chars );

value_t code( size_t n );
index_t code_s( size_t n );

value_t table( size_t n, ...);
index_t table_s( size_t n, value_t *keys );

value_t boolean( int x );
value_t symbol( char *name );
value_t fixnum( long x );

// safecasts ------------------------------------------------------------------
cons_t   *tocons( const char *fname, value_t x );
symbol_t *tosymbol( const char *fname, value_t x );
fixnum_t  tofixnum( const char *fname, value_t x );
builtin_t tobuiltin( const char *fname, value_t x );

// stack manipulation ---------------------------------------------------------
index_t push( value_t x );
index_t pushn( size_t n );
index_t push_s( const char *fname, value_t x );
index_t pushn_s(const char *fname, value_t x );

value_t pop( void );
value_t popn( size_t n );
value_t pop_s( const char *fname );
value_t popn_s( const char *fname, size_t n );

// error handlign -------------------------------------------------------------
void    error(const char *fname, const char *fmt, ...);
void    require(const char *fname, bool test, const char *fmt, ...);

// convenience macros ---------------------------------------------------------
#define ascons(x) ((cons_t*)pval(x))

#define car(x) get(cons, x, car)
#define cdr(x) get(cons, x, cdr)

#define car_s(f, x) get_s(f, cons, x, car)
#define cdr_s(f, x) get_s(f, cons, x, cdr)

#define asvector(x) ((vector_t*)pval(x))
#define asstring(x) ((string_t*)pval(x))

#define adata(x)   (((void**)pval(x))[1])
#define alen(x)    get(vector, x, length)
#define asize(x)   get(vector, x, size)
#define aflags(x)  get(vector, x, flags)
#define atype(x)   ival(aflags(x))
#define vdata(x)   ((value_t*)adata(x))
#define sdata(x)   ((char*)adata(x))
#define cdata(x)   ((short*)adata(x))

#define assymbol(x) ((symbol_t*)pval(x))

#define sbind(x) get( symbol, x, bind )
#define sname(x) get( symbol, x, name )
#define shash(x) get( symbol, x, hash )
#define sidno(x) get( symbol, x, idno )

#define sbind_s(f, x) get_s( f, symbol, x, bind )
#define sname_s(f, x) get_s( f, symbol, x, name )
#define shash_s(f, x) get_s( f, symbol, x, hash )
#define sidno_s(f, x) get_s( f, symbol, x, idno )

#define asclosure(x) ((closure_t*)pval(x))

#define clname(x)  get(closure, x, name)
#define clenvt(x)  get(closure, x, envt)
#define clvals(x)  get(closure, x, vals)
#define clcode(x)  get(closure, x, code)

#define astable(x) ((table_t*)pval(x))

#define tbkey(x)    get(table, x, key)
#define tbbind(x)   get(table, x, bind)
#define tbhash(x)   get(table, x, hash)
#define tbleft(x)   get(table, x, left)
#define tbright(x)  get(table, x, right)
#define tbparent(x) get(table, x, parent)

#endif
