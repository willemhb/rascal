#ifndef rascal_atom_h
#define rascal_atom_h

#include "object.h"

// symbol and symbol table
struct symbol_t
{
  object_t  object;

  symbol_t *left, *right;

  flags_t   isGensym  : 16;
  flags_t   isKeyword : 16;
  arity_t   length;

  idno_t    idno;
  hash_t    hash;
  char_t    name[0];
};

// forward declarations
// symbol api
symbol_t *new_symbol( size_t n );
void      init_symbol( symbol_t *symbol, char *name, hash_t hash, idno_t idno, bool isGensym, bool isKeyword, arity_t length );
void      mark_symbol( object_t *object );
void      free_symbol( object_t  *object );
size_t    symbol_size( object_t *object );
hash_t    hash_symbol( object_t *object );

symbol_t *mk_symbol( char *name, hash_t hash, bool isGensym );
symbol_t *intern_symbol( char *name );
value_t   symbol( char *name );
value_t   gensym( char *name );

// gc and initialization
void      symbol_init( void );

// convenience
#define as_sym(val)        ((symbol_t*)as_ptr(val))
#define sym_hash(val)      (as_sym(val)->hash)
#define sym_idno(val)      (as_sym(val)->idno)
#define sym_name(val)      (as_sym(val)->name)
#define sym_length(val)    (as_sym(val)->length)
#define sym_isGensym(val)  (as_sym(val)->isGensym)
#define sym_isKeyword(val) (as_sym(val)->isKeyword)
#define sym_left(val)      (as_sym(val)->left)
#define sym_right(val)     (as_sym(val)->right)

#endif
