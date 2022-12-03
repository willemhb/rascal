#ifndef rl_obj_string_h
#define rl_obj_string_h

#include "vm/object.h"

#include "vm/obj/support/string.h"

/* commentary

   The builtin Rascal string type. */

/* C types */
struct rl_string_t
{
  object_t obj;
  string_t string;
};

/* globals */

extern datatype_t StringType;

/* API */
rl_string_t  make_rl_string( const char *chars );
rl_string_t  make_rl_string_from_symbol( symbol_t *symbol );
symbol_t    *make_symbol_from_rl_string( rl_string_t *string );
glyph_t      get_rl_string_ref( rl_string_t *string, long i );
size_t       get_rl_string_len( rl_string_t *string );

/* runtime dispatch */
void rl_obj_string_init( void );
void rl_obj_string_mark( void );
void rl_obj_string_cleanup( void );

/* convenience */

#endif
