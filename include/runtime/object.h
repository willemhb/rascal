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

#define maptype(x)  (asroot(x)->type)
#define mapdata(x)  (asroot(x)->data)
#define maplen(x)   (asroot(x)->len)
#define mapcap(x)   (asroot(x)->cap)

#define mapright(x) (asnode(x)->right)
#define mapleft(x)  (asnode(x)->left)
#define mapbind(x)  (asnode(x)->bind)
#define mapkey(x)   (asnode(x)->key)
#define mapval(x)   (asnode(x)->val)
#define mapptr(x)   (asnode(x)->ptr)

#define assymbol(x) ((symbol_t*)asptr(x))
#define symname(x)  (&((assymbol(x)->name)[0]))
#define symbind(x)  (assymbol(x)->bind)
#define symidno(x)  (assymbol(x)->idno)

#define asport(x)     ((port_t*)asptr(x))
#define portvalue(x)  (asport(x)->value)
#define portbuffer(x) (asport(x)->buffer)
#define portname(x)   (&((asport(x)->name)[0]))

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
value_t new_binary( int n, Ctype_t e );
value_t new_string( int n );
value_t new_bytecode( int n );
value_t new_port( int n );
value_t new_closure( void );

// higher-level constructors --------------------------------------------------
value_t mk_cons( value_t ca, value_t cd );
value_t get_symbol( char *s, int n );
value_t mk_symbol( char *s, int n );
value_t mk_vector( int n, value_t *a );
value_t mk_binary( int n, Ctype_t e, void *b );
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
value_t   vector_ref( vector_t *x, int n );
vector_t *vector_set( vector_t *x, int n, value_t v );
vector_t *vector_put( vector_t *x, value_t v );

char      string_ref( binary_t *x, int n );
vector_t *string_set( binary_t *x, int n, char c );
vector_t *string_put( binary_t *x, char c );

ushort     code_ref( binary_t *x, int n );
bytecode_t *code_set( binary_t *x, int n, ushort i );
bytecode_t *code_put( binary_t *x, ushort i );

#endif
