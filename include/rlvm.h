#ifndef rascal_rlvm_h
#define rascal_rlvm_h

#include "object.h"

struct closure_t
{
  OBJHEAD;

  lambda_t      *template;
  environment_t *cenvt;
};

struct environment_t
{
  OBJHEAD;

  bool           is_lenvt;
  bool           is_cenvt;
  environment_t *next;

  union
  {
    vector_t    *names;
    vector_t    *binds;
  };
};

struct control_t
{
  OBJHEAD;

  control_t     *next;        // continuation
  environment_t *environment; // current environment
  value_t       *locals;      // local environment frame
  lambda_t      *function;    // executing function
  ushort        *ip;          // program counter
};

/* globals */
/* VM */
extern vector_t Stack;

// APIs
control_t *make_control( closure_t *module, control_t *next );

/* utilities/convenience */
value_t  *rl_push( value_t x );
value_t   rl_pop( void );
value_t  *rl_pushn( size_t n );
value_t   rl_popn( size_t n );
value_t   rl_peek( size_t n );

#endif
