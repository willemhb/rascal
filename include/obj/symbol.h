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
  value_t  bind; /* toplevel binding -- will eventually be superceded by toplevel namespaces */
};

/* globals */
extern datatype_t SymbolType;

/* API */
symbol_t *make_symbol( char *name );
symbol_t *intern_string( char *name );
value_t   symbol( char *name );

/* runtime */
void rl_obj_symbol_init( void );
void rl_obj_symbol_mark( void );

/* convenience */
#define is_symbol( x ) ((x)==NUL)
#define as_symbol( x ) ((symbol_t*)as_object(x))

#endif
