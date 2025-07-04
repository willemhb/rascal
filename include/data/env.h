#ifndef rl_data_env_h
#define rl_data_env_h

/* Object representing compile-time and run-time environments. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define as_env(x)        ((Env*)as_obj(x))
#define is_local_env(e)  ((e)->parent != NULL)
#define is_global_env(e) ((e)->parent == NULL)

// C types --------------------------------------------------------------------
struct Env {
  HEAD;

  Env*   parent;

  int    arity;
  int    ncap; // number of captured *local* upvalues

  EMap  vars; // personal namespace

  union {
    EMap  upvs;
    Exprs vals;
  };
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Env* mk_env(Env* parent);

// initialization -------------------------------------------------------------
void toplevel_init_data_env(void);

#endif
