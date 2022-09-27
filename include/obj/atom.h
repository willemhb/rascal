#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj.h"
#include "obj/table.h"

// C types --------------------------------------------------------------------
typedef struct str_t str_t;

typedef enum
  {
    atom_fl_gensym =1,
    atom_fl_keyword=2,
  } atom_fl_t;

typedef struct atom_t
{
  OBJ_HEAD
  str_t     *name;
  hash_t     hash;
  idno_t     idno;
} atom_t;

typedef struct symt_t
{
  idno_t  counter;
  table_t table;
} symt_t;

// globals
extern symt_t Symbols;

// forward declarations & generics --------------------------------------------
val_t   intern( char *name );
atom_t *new_atom( char *name, hash_t h, bool hp, bool gsym );

bool   is_keyword( val_t val );
bool   is_gensym( val_t val );

// initialization
void atom_init( void );
void atom_mark( void );

// convenience ----------------------------------------------------------------
#define is_atom(x) (is_obj_type(x, atom_type))
#define as_atom(x) ((atom_t*)as_ptr(x))

#endif
