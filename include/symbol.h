#ifndef rascal_atom_h
#define rascal_atom_h

#include "table.h"

// symbol and symbol table
typedef enum
  {
    symbol_fl_keyword = 0x1,
    symbol_fl_gensym  = 0x2
  } symbol_fl_t;

struct symbol_t
{
  object_t object;
  idno_t  idno;
  hash_t  hash;
  char_t *name;
};

typedef struct symbol_table_t
{
  table_t table;
  idno_t  idno;
} symbol_table_t;

// globals
extern symbol_table_t Symbols;

extern type_t *SymbolType, *SymbolTableType;

extern value_t Quote, Error;

// forward declarations
symbol_t *new_symbol(size_t l);
void      init_symbol(symbol_t *atom, symbol_table_t *symt, char *name);
void      free_symbol(object_t *obj);
void      prin_symbol(stream_t *port, value_t value);


// external API
symbol_t *symbol( char *name );
void      symbol_init( void );

// convenience
#define is_symbol(val)     isa(val, SYMBOL)
#define as_symbol(val)     ((symbol_t*)as_ptr(val))

#define symbol_hash(val)   (as_symbol(val)->hash)
#define symbol_name(val)   (as_symbol(val)->name)
#define symbol_idno(val)   (as_symbol(val)->idno)

#endif
