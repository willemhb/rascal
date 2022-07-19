#ifndef rascal_number_h
#define rascal_number_h

#include "rascal.h"

// constructors ---------------------------------------------------------------
value_t fixnum( fixnum_t x );
value_t boolean( int x );
value_t character( int ch );

// utilities ------------------------------------------------------------------
size_t  fixnum_init( const char *fname, value_t x, Ctype_t c, void *spc );
bool    fixnum_fits( fixnum_t x, Ctype_t ctype );

// predicates -----------------------------------------------------------------
bool is_fixnum( value_t x );
bool is_boolean( value_t x );
bool is_character( value_t x );
bool is_true( value_t x );
bool is_false( value_t x );

// safecasts ------------------------------------------------------------------
fixnum_t    tofixnum( const char *fname, value_t x );
boolean_t   toboolean( const char *fname, value_t x );
character_t tocharacter( const char *fname, value_t x );

// builtins ------------------------------------------------------------------
void builtin_fixnum( size_t n );
void builtin_character( size_t n );
void builtin_boolean( size_t n );

void builtin_add( size_t n );
void builtin_sub( size_t n );
void builtin_mul( size_t n );
void builtin_div( size_t n );
void builtin_mod( size_t n );

void builtin_eqn( size_t n );
void builtin_ltn( size_t n );

void builtin_is_fixnum( size_t n );
void builtin_is_character( size_t n );
void builtin_is_boolean( size_t n );
void builtin_is_true( size_t n );
void builtin_is_false( size_t n );

// initialization -------------------------------------------------------------
void number_init( void );

#endif
