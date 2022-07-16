#ifndef rascal_builtin_h
#define rascal_builtin_h

#include "common.h"
#include "types.h"

// declarations/implementations for builtin functions -------------------------
void r_cons( size_t n );
void r_consp( size_t n );
void r_listp( size_t n );
void r_car( size_t n );
void r_cdr( size_t n );
void r_xar( size_t n );
void r_xdr( size_t n );
void r_ncat( size_t n );
void r_nrev( size_t n );
void r_assoc( size_t n );

void r_vector( size_t n );
void r_vectorp( size_t n );
void r_len( size_t n );
void r_nth( size_t n );
void r_xth( size_t n );
void r_put( size_t n );

void r_string( size_t n );
void r_stringp( size_t n );

void r_symbol( size_t n );
void r_gensym( size_t n );
void r_symbolp( size_t n );
void r_gensymp( size_t n );

void r_functionp( size_t n );
void r_builtinp( size_t n );
void r_closurep( size_t n );

void r_idp( size_t n );

void r_add( size_t n );
void r_sub( size_t n );
void r_mul( size_t n );
void r_div( size_t n );
void r_rem( size_t n );
void r_eqp( size_t n );
void r_ltp( size_t n );

void r_exec( size_t n );
void r_eval( size_t n );
void r_comp( size_t n );
void r_compf( size_t n );
void r_apply( size_t n );
void r_read( size_t n );
void r_prin( size_t n );
void r_load( size_t n );

// initialization -------------------------------------------------------------
value_t mk_builtin( char *name, builtin_t callback );
void mk_builtins( void );

#endif
