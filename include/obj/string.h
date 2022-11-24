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

/* initialization */
void rl_obj_string_init( void );

/* convenience & utilities */
#define as_string( x ) ((string_t*)as_obj(x))
#define is_string( x ) value_is_type(x, &StringType)

#endif
