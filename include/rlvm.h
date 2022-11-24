#ifndef rascal_rlvm_h
#define rascal_rlvm_h

#include "object.h"

struct closure_t
{
  OBJHEAD;

  lambda_t      *function;
  environment_t *environment;
};

struct environment_t
{
  OBJHEAD;

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
  lambda_t      *function;    // executing function
  ushort        *ip;          // program counter
};

#define is_closure( x )      value_is_type(x, &ClosureType)
#define as_closure( x )     ((closure_t*)as_obj(x))
#define is_environment( x )  value_is_type(x, &EnvironmentType)
#define as_environment( x ) ((environment_t*)as_obj(x))
#define is_control( x )      value_is_type(x, &ControlType)
#define as_control( x )     ((control_t*)as_obj(x))

#define RL_VM_STACK_SIZE UINT16_MAX

// miscellaneous vm state
typedef struct vm_t
{
  char      *error;       // error message
  bool       panic_mode;  // panicing?
  control_t *control;     // currently executing function or module
  closure_t *toplevel;    // toplevel module (at the moment just stores global bindings/namespace)
} vm_t;

/* globals */
/* VM */
extern vm_t Vm;

/* apis */
/* control */
control_t     *make_control( void );
void           init_control( control_t *control, closure_t *module, control_t *caller, size_t n );
void           free_control( control_t *control );

control_t     *make_call_frame( control_t *caller, closure_t *module, size_t n );
void           free_call_frame( control_t **returner );

/* environment*/
environment_t *make_environment( void );
void           init_environment( environment_t *environment, environment_t *parent, size_t n );
void           free_environment( environment_t *environment );

value_t        envt_ref( environment_t *environment, size_t i, size_t j );
value_t        envt_set( environment_t *environment, size_t i, size_t j, value_t value );
size_t         envt_bind( environment_t *environment, value_t value );
value_t       *envt_at( long i );

/* closure */
closure_t     *make_closure( void );
void           init_closure( closure_t *closure, lambda_t *lambda, environment_t *environment );
void           free_closure( closure_t *closure );

/* utilities/convenience */
size_t  rl_push( value_t x );
value_t rl_pop( void );
size_t  rl_pushn( size_t n );
value_t rl_popn( size_t n );

value_t rl_peek( long i );

void      rl_panic( const char *fmt, ... );
bool      rl_recover( void );

/* initialization */
void      rl_rlvm_init( void );

#endif
