#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj.h"

// C types --------------------------------------------------------------------
typedef struct symt_t    symt_t;
typedef struct symt_kv_t symt_kv_t;
typedef struct str_t     str_t;

DECL_OBJ(atom);

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

// hashing utilities ----------------------------------------------------------
hash_t hash_string( char *chars );
hash_t hash_bytes( uint8_t *mem, arity_t cnt );
hash_t hash_int( uint64_t u );
hash_t hash_real( real_t r );
hash_t hash_ptr( ptr_t p );
hash_t mix_hashes( hash_t h1, hash_t h2 );

#endif
