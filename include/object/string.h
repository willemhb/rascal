#ifndef rascal_string_h
#define rascal_string_h

#include "rascal.h"
#include "ctypes.h"
#include "strutils.h"
#include "describe/array.h"
#include "describe/safety.h"

struct string_t   ArrayType( char_t );
struct string_s_t SharedArrayType( char_t );

#define asstr(x)   ((string_t*)ptr(x))
#define slen(x)    (asstr(x)->len)
#define sctype(x)  (asstr(x)->ctype)
#define senc(x)    (asstr(x)->flags)
#define scap(x)    (asstr(x)->cap)
#define schars(x)  (&((asstr(x)->space)[0]))
#define sdata(x)   (asstr(x)->data)

DeclareSafeCast( string, string_t* );
DeclareArraySize( string );
DeclareArrayData( string, character_t );
DeclareArrayParent( string );
DeclareArrayRef( string, character_t );
DeclareArrayXef( string, character_t );
DeclareArrayPut( string, character_t );
DeclareArrayXut( string, character_t );
DeclareArrayResize( string );

#define tostring( x )                     GenericSafeCast( string, x )
#define stringp( x )                      GenericTypeP( string )

#define asize_string( x )                 GenericArraySize( string, x )
#define data_string( x )                  GenericArrayData( string, x )
#define parent_string( x )                GenericArrayParent( string, x )
#define ref_string( x, i )                GenericArrayRef( string, x, i )
#define xef_string( x, c, i, ch )         GenericArrayXef( string, x, c, i, ch )
#define put_string( x, ch )               GenericArrayPut( string, x, ch )
#define xut_string( x, c, ch )            GenericArrayXut( string, x, c, ch )
#define resize_string( x, n )             GenericArrayResize( string, x, n )

// api ------------------------------------------------------------------------
value_t new_string(char_t *chars, flags_t fl );
value_t share_string( object_t *ob, flags_t fl, index_t off );

// internal methods -----------------------------------------------------------
size_t  print_string( FILE *ios, value_t xs );

// initialization -------------------------------------------------------------
void string_init( void );

#endif
