#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj.h"

// C types --------------------------------------------------------------------
typedef struct sym_table_t    sym_table_t;
typedef struct sym_table_kv_t sym_table_kv_t;
typedef struct str_t          str_t;

typedef struct atom_t
{
  OBJ_HEAD;
  obj_t     *name;
  hash_t     hash;
  idno_t     idno;
} atom_t;

// globals --------------------------------------------------------------------
extern sym_table_t *SymbolTable;
extern idno_t SymbolCount;

// forward declarations & generics --------------------------------------------
obj_t *intern( char *name );

#define atom(arg)				\
  _Generic((x),					\
	   char*:char_atom,			\
	   obj_t*:obj_atom,			\
	   str_t*:str_atom,			\
	   )(arg)

obj_t *char_atom( char *name );
obj_t *obj_atom( obj_t *str );
obj_t *str_atom( str_t *str );

// hashing utilities ----------------------------------------------------------
hash_t hash_string( char *chars );
hash_t hash_bytes( uint8_t *mem, arity_t cnt );

// toplevel dispatch ----------------------------------------------------------
void atom_mark( void );
void atom_init( void );

// convenience ----------------------------------------------------------------

#endif
