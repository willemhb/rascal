#ifndef rascal_runtime_object_h
#define rascal_runtime_object_h

#include "rascal.h"
#include "ctypes.h"

// utility macros ------------------------------------------------------------
#define ascons(x) ((cons_t*)asptr(x))
#define car(x)    (ascons(x)->car)
#define cdr(x)    (ascons(x)->cdr)

#define obflags(x) (imflags(car(x)))
#define obtype(x)  (imtype(car(x)))

#define asvec(x)  ((vector_t*)asptr(x))
#define vlen(x)   (asvec(x)->len)
#define vcap(x)   (asvec(x)->cap)
#define vtype(x)  (asvec(x)->type)
#define vdata(x)  (asvec(x)->data)

#define asbin(x)  ((binary_t*)asptr(x))
#define btype(x)  (asbin(x)->type)
#define blen(x)   (asbin(x)->len)
#define bcap(x)   (asbin(x)->cap)
#define bdata(x)  (asbin(x)->data)

#define asnode(x)   ((node_t*)asptr(x))
#define asroot(x)   ((root_t*)asptr(x))

#define mtype(x)  (asroot(x)->type)
#define mdata(x)  (asroot(x)->data)
#define mlen(x)   (asroot(x)->len)
#define mcap(x)   (asroot(x)->cap)

#define mright(x) (asnode(x)->right)
#define mleft(x)  (asnode(x)->left)
#define mbind(x)  (asnode(x)->bind)
#define mkey(x)   (asnode(x)->key)
#define mval(x)   (asnode(x)->val)
#define mptr(x)   (asnode(x)->ptr)
#define mhash(x)    (asnode(x)->hash)

#define assymbol(x) ((symbol_t*)asptr(x))
#define symname(x)  (&((assymbol(x)->name)[0]))
#define symbind(x)  (assymbol(x)->bind)
#define symidno(x)  (assymbol(x)->idno)

#define asport(x)     ((port_t*)asptr(x))
#define pvalue(x)  (asport(x)->value)
#define pbuffer(x) (asport(x)->buffer)
#define pname(x)   (&((asport(x)->name)[0]))

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

bool arrayp( value_t x );
bool listp( value_t x );
bool rootp( value_t x );
bool nodep( value_t x );
bool boxedp( value_t x );

// utilities -----------------------------------------------------------------
size_t  sym_sizeof( value_t x );
size_t  arr_sizeof( value_t x );
size_t  val_sizeof( value_t x );

int     val_order( value_t x, value_t y );
hash_t  val_hash( value_t x );

type_t  val_typeof( value_t x );
char   *val_typename( value_t x );
Ctype_t val_ctype( value_t x );
Ctype_t val_eltype( value_t x );

// constructors ---------------------------------------------------------------
value_t new_cons( int n );
value_t new_symbol( char *s, int n, hash_t h, bool i );
value_t new_vector( int n );
value_t new_table( int n );
value_t new_dict( int n );
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
value_t mk_instruction( instruction_t i );

// array accessors ------------------------------------------------------------
value_t   vector_ref( value_t x, int n );
value_t   vector_set( value_t x, int n, value_t v );
value_t   vector_put( value_t x, value_t v );
value_t   vector_resize( value_t x, int n );

char      string_ref( value_t x, int n );
value_t   string_set( value_t x, int n, char c );
value_t   string_put( value_t x, char c );
value_t   string_resize( value_t x, int n );

ushort    code_ref( value_t x, int n );
value_t   code_set( value_t x, int n, ushort i );
value_t   code_put( value_t x, ushort i );
value_t   code_resize( value_t x, int n );

// table accessors ------------------------------------------------------------
value_t   dict_ref( value_t x, value_t k );
value_t   dict_set( value_t x, value_t k, value_t b );
value_t   dict_put( value_t x, value_t k );

#endif
