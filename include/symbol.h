#ifndef rascal_symbol_h
#define rascal_symbol_h

#include "rascal.h"

// flags ----------------------------------------------------------------------
typedef enum {
  symfl_constant = 0x001,
  symfl_syntax   = 0x002
} symflags_t;

// globals --------------------------------------------------------------------
extern value_t r_kw_generate, r_kw_intern;

// constructors ---------------------------------------------------------------
value_t symbol( char *name );
index_t symbol_s( char *fname );
value_t gensym( char *name );
index_t gensym_s( char *name );

// predicates -----------------------------------------------------------------
bool is_symbol( value_t x );
bool is_gensym( value_t x );
bool is_keyword( value_t x );
bool is_bound( value_t x );

// safecasts -----------------------------------------------------------------
symbol_t *tosymbol( const char *fname, value_t x );

// methods -------------------------------------------------------------------
size_t symbol_prin( FILE *ios, value_t x );
int    symbol_order( value_t x, value_t y );
hash_t symbol_hash( value_t x );

// builtins ------------------------------------------------------------------
void builtin_is_symbol( size_t n );
void builtin_is_gensym( size_t n );
void builtin_is_keyword( size_t n );
void builtin_is_bound( size_t n );

// initialization -------------------------------------------------------------
void symbol_init( void );

#define assymbol(x)  ((symbol_t*)pval(x))

#define sname(x) (&(get(symbol, x, name)[0]))
#define sidno(x) get(symbol, x, idno)
#define sbind(x) get(symbol, x, bind)

#define sname_s(f, x) get_s(f, symbol, x, name)
#define sidno_s(f, x) get_s(f, symbol, x, idno)
#define sbind_s(f, x) get_s(f, symbol, x, bind)

#define init_symbol(o, h, n, l)						\
  do {									\
    init_ob(o,								\
	    tag_symbol,							\
	    C_uint8,							\
	    enc_utf8,							\
	    false,							\
	    sizeof(symbol_t)+l+1);					\
    									\
    ((symbol_t*)o)->idno = Symcnt++;					\
    ((symbol_t*)o)->bind = val_unbound;					\
    ((symbol_t*)o)->hash = h;						\
  } while (0)

#endif
