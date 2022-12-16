#ifndef rl_vm_obj_support_envt_h
#define rl_vm_obj_support_envt_h

#include "rascal.h"

#include "vm/obj/support/vector.h"

/* commentary

   Runtime envt representation.

   Corresponds with a namespace. */

/* C types */
typedef struct envt_t envt_t;

struct envt_t
{
  envt_t   *next;
  vector_t  binds;
};

/* globals */

/* API */
envt_t *make_envt( void );
void    init_envt( envt_t *envt, envt_t *parent, size_t n_binds, rl_value_t *binds );
void    free_envt( envt_t *envt );

/* accessors */
envt_t   *get_envt_next( envt_t *envt );
vector_t  get_envt_binds( envt_t *envt );
rl_value_t   get_envt_ref( envt_t *envt, size_t i, size_t j );
rl_value_t   set_envt_ref( envt_t *envt, size_t i, size_t j, rl_value_t bind );

/* runtime dispatch */
void rl_vm_obj_support_envt_init( void );
void rl_vm_obj_support_envt_mark( void );
void rl_vm_obj_support_envt_cleanup( void );

/* convenience */

#endif
