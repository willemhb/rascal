#ifndef rl_obj_string_h
#define rl_obj_string_h

#include "obj/object.h"
#include "tpl/decl/array.h"

/* C types */
struct string_t { ARRAY(char); };

/* globals */
extern type_t StringType;

/* API */
void resize_string( string_t *string, size_t new_size );

/* runtime */
void rl_obj_string_init( void );
void rl_obj_string_mark( void );

/* convenience */
static inline bool      is_string( value_t x ) { return rl_isa(x, &StringType); }
static inline string_t *as_string( value_t x ) { return (string_t*)as_object(x); }

#endif
