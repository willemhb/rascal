#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj/object.h"
#include "obj/array.h"
#include "obj/type.h"

// symbol and symbol table
struct symbol_t
{
  object_t  object;
  symbol_t *next;
  hash_t    hash;
  char     *name;
  idno_t    idno;
  bool      gensym;
  bool      keyword;
};

struct symbols_t
{
  TABLE_SLOTS(symbol_t*);
};

// globals
extern symbols_t Symbols;

extern type_t *SymbolType, *SymbolsType;

// forward declarations
// symbol api
value_t   symbol( char *name );
value_t   gensym( char *name );

// gc and initialization
void      symbol_init( void );

static inline void symbol_mark( void )
{
  rl_mark( (boxed_t*)SymbolType );
  rl_mark( (boxed_t*)SymbolsType );
  rl_mark( (boxed_t*)&Symbols );
}

static inline void symbol_unmark( void )
{
  rl_unmark( (boxed_t*)SymbolType );
  rl_unmark( (boxed_t*)SymbolsType );
  rl_unmark( (boxed_t*)&Symbols );
}

static inline void symbol_types_init( void )
{
  extern dtype_t    SymbolTypeOb, SymbolsTypeOb;
  extern function_t SymbolFunc;

  SymbolType  = &SymbolTypeOb.type;  SymbolType->constructor = &SymbolFunc;
  SymbolsType = &SymbolsTypeOb.type;
}

// convenience
#define as_sym(val)        ((symbol_t*)as_ptr(val))
#define is_sym(val)        is_repr(val, SYMBOL)

#endif
