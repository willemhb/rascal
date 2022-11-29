#ifndef rl_obj_symbol_h
#define rl_obj_symbol_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
struct symbol_t
{
  object_t obj;
  string_t name;
  ulong    hash;
  ulong    idno;
};

/* globals */
extern datatype_t SymbolType;

/* API */
symbol_t *symbol( char *name );
symbol_t *gensym( char *name );

/* runtime */
void rl_obj_nul_init( void );
void rl_obj_nul_mark( void );

/* convenience */
#define is_nul( x ) ((x)==NUL)
#define as_nul( x ) (NULL)

#endif
