#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj.h"

// C types --------------------------------------------------------------------
typedef struct symt_t    symt_t;
typedef struct symt_kv_t symt_kv_t;
typedef struct str_t     str_t;

DECL_OBJ(atom);
DECL_OBJ_API(atom);

struct atom_t
{
  OBJ_HEAD;
  str_t     *name;
  hash_t     hash;
  idno_t     idno;
};

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

bool   is_keyword( val_t val );
bool   is_gensym( val_t val );

// convenience ----------------------------------------------------------------
#define is_atom(x) (is_obj_type(x, atom_type))

#endif
