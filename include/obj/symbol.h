#ifndef rascal_obj_symbol_h
#define rascal_obj_symbol_h

#include "obj/table.h"

// symbol and symbol table implemenations
typedef struct symbol_t    symbol_t;
typedef struct symbols_t   symbols_t;
typedef struct atom_impl_t atom_impl_t;

struct symbol_t
{
  OBJECT
  hash_t  hash;
  char   *name;
  idno_t  idno;
  bool   gensym;
  bool   keyword;
};

struct symbols_t
{
  TABLE;
};

struct symbol_impl_t
{
  OBJECT

  idno_t     counter; // the id for the type
  symbols_t *table;   // the symbol table for the type
};

// globals
extern symbols_t Symbols;
extern type_t SymbolType, SymbolsType, AtomImplType;

// forward declarations
object_t *symbol( char *name );
object_t *gensym( char *name );

// register
void rl_init_obj_symbol( void );
void rl_mark_obj_symbol( void );
void rl_unmark_obj_symbol( void );
void rl_finalize_obj_symbol( void );

// convenience
#define is_symbol( x )   is_type( x, &SymbolType )
#define as_sym( x )      ( (symbol_t*)as_obj( x ) )

#define sym_name( x )    ( as_sym( x )->name )
#define sym_hash( x )    ( as_sym( x )->hash )
#define sym_idno( x )    ( as_sym( x )->idno )
#define sym_gensym( x )  ( as_sym( x )->gensym )
#define sym_keyword( x ) ( as_sym( x )->keyword )

#endif
