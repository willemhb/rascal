#ifndef rl_obj_symbol_h
#define rl_obj_symbol_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
struct rl_symbol_t
{
  rl_object_t obj;
  string_t    name;
  ulong       hash;
  ulong       idno;
};

/* globals */
extern rl_datatype_t SymbolType;

/* API */
/* constructors */
rl_symbol_t *make_symbol( const char *name );
rl_value_t   symbol( const char *name );

/* accessors */
string_t get_symbol_name( rl_symbol_t *symbol );
ulong    get_symbol_hash( rl_symbol_t *symbol );
ulong    get_symbol_idno( rl_symbol_t *symbol );

/* runtime */
void rl_obj_symbol_init( void );
void rl_obj_symbol_mark( void );
void rl_obj_symbol_cleanup( void );

/* convenience */
#define is_symbol( x ) ((x)==NUL)
#define as_symbol( x ) ((rl_symbol_t*)as_object(x))

/* toplevel define helper */
void define( char *name, rl_value_t value );

#endif
