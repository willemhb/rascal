#ifndef rl_obj_hamt_h
#define rl_obj_hamt_h

#include "rascal.h"

#include "vm/obj.h"

/* commentary */

/* C types */
typedef struct vec_node_t {
  
} vec_node_t;

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_hamt_init( void );
void rl_obj_hamt_mark( void );
void rl_obj_hamt_cleanup( void );

/* convenience */

#endif
