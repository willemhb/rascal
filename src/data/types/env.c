
/* Object representing compile-time and run-time environments. */
// headers --------------------------------------------------------------------
#include "data/types/env.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_env(void* ptr);
void free_env(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

static void trace_emap(EMap* m) {
  for ( int i=0, j=0; i < m->max_count && j < m->count; i++ ) {
    EMapKV* kv = &m->kvs[i];

    if ( kv->key != NULL ) {
      j++;
      mark_obj(kv->key);
      mark_obj(kv->val);
    }
  }
}

void trace_env(void* ptr) {
  Env* e = ptr;

  mark_obj(e->parent);

  trace_emap(&e->vars);

  if ( is_local_env(e) )
    trace_emap(&e->upvs);

  else
    trace_exprs(&e->vals);
}

void free_env(void* ptr) {
  Env* e = ptr;

  free_emap(&e->vars);

  if ( is_local_env(e) )
    free_emap(&e->upvs);

  else
    free_exprs(&e->vals);
}

// external -------------------------------------------------------------------
Env* mk_env(Env* parent) {
  Env* env = mk_obj(EXP_ENV, 0);

  env->parent = parent;
  env->arity  = 0;
  env->ncap   = 0;
  init_emap(&env->vars);
  init_emap(&env->upvs);

  return env;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_env(void) {
  Types[EXP_ENV] = (ExpTypeInfo) {
    .type     = EXP_ENV,
    .name     = "env",
    .obsize   = sizeof(Env),
    .trace_fn = trace_env,
    .free_fn  = free_env  
  };
}
