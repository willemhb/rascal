#include <assert.h>

#include "vm/memory.h"

#include "vm/obj/support/vector.h"
#include "vm/obj/support/envt.h"

/* commentary */

/* C types */

/* globals */

/* API */
/* memory API */
envt_t *make_envt( void )
{
  return alloc(sizeof(envt_t));
}

void    init_envt( envt_t *envt, envt_t *parent, size_t n_binds, value_t *binds )
{
  envt->next  = parent;
  envt->binds = make_vector(n_binds, binds);
}

void    free_envt( envt_t *envt )
{
  free_vector(envt->binds);
  dealloc(envt, sizeof(envt_t));
}

/* accessors */
#include "tpl/impl/record.h"
GET(envt, next, envt_t*);
GET(envt, binds, vector_t);

value_t get_envt_ref( envt_t *envt, size_t i, size_t j )
{
  while ( i-- )
    envt = get_envt_next(envt);

  return vector_ref(get_envt_binds(envt), j);
}

value_t set_envt_ref( envt_t *envt, size_t i, size_t j, value_t x )
{
  while ( i-- )
    envt = get_envt_next(envt);

  return vector_set(get_envt_binds(envt), j, x);
}

/* runtime dispatch */
void rl_vm_obj_support_envt_init( void )    {}
void rl_vm_obj_support_envt_mark( void )    {}
void rl_vm_obj_support_envt_cleanup( void ) {}

