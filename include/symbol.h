#ifndef rascal_atom_h
#define rascal_atom_h

#include "cvalue.h"

// symbol and symbol table
typedef enum
  {
    symbol_fl_keyword = 0x1,
    symbol_fl_gensym  = 0x2
  } symbol_fl_t;

struct symbol_t
{
  object_t object;
  symbol_t *left, *right;

  idno_t  idno;
  hash_t  hash;
  char_t *name;
};

typedef struct
{
  object_t  object;
  symbol_t *symbols;
  idno_t    counter;
} symbol_table_t;

// globals
extern symbol_table_t *Symbols;

extern value_t Quote, Error;

// forward declarations
symbol_t *new_symbol(size_t l);
void      init_symbol(symbol_t *atom, symbol_table_t *symt, char *name, hash_t hash);
void      free_atom(object_t *obj);
void      prin_atom(stream_t *port, value_t value);


// external API
value_t     symbol( char *name );
void      atom_init( void );

// convenience
#define is_atom(val)     isa(val, ATOM)
#define as_atom(val)     ((atom_t*)as_ptr(val))

#define atom_hash(val)   (as_atom(val)->hash)
#define atom_name(val)   (as_atom(val)->name)
#define atom_idno(val)   (as_atom(val)->idno)

#endif
