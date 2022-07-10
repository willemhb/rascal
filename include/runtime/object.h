#ifndef rascal_runtime_object_h
#define rascal_runtime_object_h

#include "rascal.h"
#include "ctypes.h"

// utility macros ------------------------------------------------------------
#define obslots(x)  ((value_t*)asptr(x))
#define ascons(x)   ((cons_t*)asptr(x))
#define car(x)      (((value_t*)asptr(x))[0])
#define cdr(x)      (((value_t*)asptr(x))[1])

#define asob(x)     ((object_t*)asptr(x))
#define obdata(x)   (((void**)asptr(x))[0])
#define obsize(x)   (asob(x)->size)
#define obflags(x)  (asob(x)->flags)
#define obtype(x)   (asob(x)->type)

#define astup(x)    ((tuple_t*)asptr(x))
#define tdata(x)    (astup(x)->data)
#define tlen(x)     (astup(x)->len)
#define tcap(x)     (astup(x)->cap)

#define asvec(x)    ((vector_t*)asptr(x))
#define vlen(x)     (asvec(x)->len)
#define vcap(x)     (asvec(x)->cap)
#define vtype(x)    (asvec(x)->type)
#define vdata(x)    (asvec(x)->data)

#define asstring(x) ((string_t*)asptr(x))
#define sdata(x)    (asstring(x)->data)

#define asnode(x)   ((node_t*)asptr(x))
#define asroot(x)   ((root_t*)asptr(x))

#define mtype(x)    (asroot(x)->type)
#define mdata(x)    (asroot(x)->data)
#define mlen(x)     (asroot(x)->len)
#define mcap(x)     (asroot(x)->cap)

#define mright(x)   (asnode(x)->right)
#define mleft(x)    (asnode(x)->left)
#define mbind(x)    (asnode(x)->bind)
#define mkey(x)     (asnode(x)->key)
#define mval(x)     (asnode(x)->val)
#define mptr(x)     (asnode(x)->ptr)
#define mhash(x)    (asnode(x)->hash)

#define assymbol(x) ((symbol_t*)asptr(x))
#define sname(x)    (assymbol(x)->data)
#define sbind(x)    (assymbol(x)->bind)
#define shash(x)    (assymbol(x)->hash)

#define asport(x)   ((port_t*)asptr(x))
#define pval(x)     (asport(x)->value)
#define pbuf(x)     (asport(x)->buffer)
#define pios(x)     (asport(x)->ios)

#define pname(x)    (&((asport(x)->name)[0]))

// predicates ----------------------------------------------------------------
bool immediatep( value_t x );
bool objectp( value_t x );
bool movedp( value_t x );

bool consp( value_t x );
bool symbolp( value_t x );
bool portp( value_t x );
bool closurep( value_t x );	    
bool vectorp( value_t x );
bool tablep( value_t x );
bool binaryp( value_t x );

bool listp( value_t x );
bool rootp( value_t x );
bool nodep( value_t x );
bool boxedp( value_t x );

// type/tag dispatching methods -----------------------------------------------
type_t val_typeof( value_t x );
size_t val_sizeof( value_t x );
size_t val_print( FILE *ios, value_t x );
int val_order( value_t x, value_t y );
hash_t val_hash( value_t x );
char *val_typename( value_t x );

// constructors ---------------------------------------------------------------
value_t new_cons( int n );
value_t new_symbol( char *s, int n, hash_t h, bool i );
value_t new_vector( int n );
value_t new_table( int n );
value_t new_string( int n );
value_t new_bytecode( int n );
value_t new_port( int n );
value_t new_closure( void );
node_t *new_node( void );

// higher-level constructors --------------------------------------------------
value_t mk_cons( value_t ca, value_t cd );
value_t get_symbol( char *s, int n );
value_t mk_symbol( char *s, int n );
value_t mk_vector( int n, value_t *a );
value_t mk_string( char *s, int n );
value_t mk_dict( int n, value_t *a );
value_t mk_table( int n );
value_t mk_port( char *s, int n, FILE *ios );
value_t mk_closure( value_t name, value_t vals, value_t code, value_t envt );

value_t mk_integer( int x );
value_t mk_character( char x );
value_t mk_control( char *x );

// array accessors ------------------------------------------------------------
value_t   vector_ref( value_t x, int n );
value_t   vector_set( value_t x, int n, value_t v );
value_t   vector_put( value_t x, value_t v );
vector_t *vector_resize( vector_t *x, int n );

char      string_ref( value_t x, int n );
value_t   string_set( value_t x, int n, char c );
value_t   string_put( value_t x, char c );
string_t *string_resize( string_t *x, int n );

ushort    code_ref( value_t x, int n );
value_t   code_set( value_t x, int n, ushort i );
value_t   code_put( value_t x, ushort i );
value_t   code_resize( value_t x, int n );

// symbol & characters table --------------------------------------------------


#endif
