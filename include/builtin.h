#ifndef rascal_builtin_h
#define rascal_builtin_h

#include "common.h"
#include "types.h"

// declarations/implementations for builtin functions -------------------------
void r_cons( int n );
void r_consp( int n );
void r_car( int n );
void r_cdr( int n );
void r_xar( int n );
void r_xdr( int n );
void r_ncat( int n );
void r_nrev( int n );

void r_vec( int n );
void r_vecp( int n );
void r_len( int n );
void r_nth( int n );
void r_xth( int n );
void r_put( int n );

void r_sym( int n );
void r_symp( int n );
void r_gensymp( int n );

void r_funp( int n );
void r_builtinp( int n );

void r_idp( int n );

void r_add( int n );
void r_sub( int n );
void r_mul( int n );
void r_div( int n );
void r_rem( int n );
void r_eqp( int n );
void r_ltp( int n );

value_t mk_builtin( char *name, builtin_t callback );

void mk_builtins( void );

#endif
