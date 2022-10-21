#ifndef rascal_obj_symbol_h
#define rascal_obj_symbol_h

#include "obj/table.h"

typedef struct
{
  OBJECT
  bool   gensym;
  bool   keyword;
  idno_t idno;
  hash_t hash;

  union
  {
    char *name;
    char *key;        // for table compatibility
  };
} symbol_t;

typedef struct
{
  TABLE;
} symbols_t;

// globals
extern symbols_t Symbols;
extern type_t SymbolType, SymbolsType;

// forward declarations
object_t *symbol( char *name );
object_t *gensym( char *name );

// register
void rl_obj_symbol_init( void );
void rl_obj_symbol_mark( void );
void rl_obj_symbol_unmark( void );
void rl_obj_symbol_finalize( void );

// convenience
#define is_sym( x )      is_type( x, &SymbolType )
#define as_sym( x )      ( (symbol_t*)as_obj( x ) )

#define sym_name( x )    ( as_sym( x )->name )
#define sym_hash( x )    ( as_sym( x )->hash )
#define sym_gensym( x )  ( as_sym( x )->gensym )
#define sym_keyword( x ) ( as_sym( x )->keyword )

#endif
