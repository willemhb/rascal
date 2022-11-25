#ifndef rl_obj_map_h
#define rl_obj_map_h

#include "obj/object.h"
#include "tpl/decl/table.h"

/* commentary */

/* C types */
struct map_t { TABLE(cons_t); };

/* globals */
extern type_t MapType;

/* API */
object_t *make_map( type_t *type, size_t n );
void      init_map( object_t *object, size_t n, void *ini );

/* runtime */
void rl_obj_map_init( void );
void rl_obj_map_mark( void );

/* convenience */
static inline bool   is_map( value_t x ) { return rl_isa(x, &MapType); }
static inline map_t *as_map( value_t x ) { return (map_t*)as_object(x); }

#endif
