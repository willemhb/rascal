#include "obj/closure.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
void trace_closure(object_t *object);

datatype_t ClosureType =
  {
    { gl_datatype_head, "closure", datatype_isa, NULL },

    .vmtype=vmtype_objptr,
    .obsize=sizeof(closure_t),
    .trace=trace_closure
  };

/* API */
/* object runtime methods */
void trace_closure( object_t *object )
{
  closure_t *closure = (closure_t*)object;

  mark_object((object_t*)get_closure_function(closure));
  mark_object((object_t*)get_closure_envt(closure));
}

/* constructors */
value_t capture_closure( lambda_t *lambda, envt_t *envt )
{
  closure_t *closure = (closure_t*)make_object(&ClosureType);

  closure->function = lambda;
  closure->envt     = envt;

  return tag_object(closure);
}

/* accessors */
#include "tpl/impl/record.h"
GET(closure, function, lambda_t*);
GET(closure, envt, envt_t*);

/* runtime dispatch */
void rl_obj_closure_init( void )
{
  gl_init_type(ClosureType);
}

void rl_obj_closure_mark( void )
{
  gl_mark_type(ClosureType);
}

void rl_obj_closure_cleanup( void ) {}

