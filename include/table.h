#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"

// convenience macros
#define TABLE( E )				\
  OBJECT;					\
  Size  count;					\
  Size  capacity;				\
  E    **entries

// C types
typedef struct SymbolTable SymbolTable;
typedef struct Symbol      Symbol;

struct SymbolTable
{
  TABLE(Symbol);

  ULong counter;   // count of all symbols (including gensyms)
};

// forward declarations
// symbol table implementation
SymbolTable *makeSymbolTable( Void );
Int          freeSymbolTable( SymbolTable *table );
Void         initSymbolTable( SymbolTable *table, Size count );

Symbol      *internSymbol( Char *name );

// hashing utilities
ULong hashString( const Char *string );
ULong hashBytes( const UChar *bytes, Size nBytes );

// initialization
Void rlTableInit( Void );

// globals
extern SymbolTable TheSymbolTable;

extern Type SymbolTableType;

#endif
