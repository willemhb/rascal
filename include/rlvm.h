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

  bool           is_lenvt;
  bool           is_captured;
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
  control_t *control;     // currently executing function
  
} vm_t;

/* globals */
/* VM */
extern vm_t Vm;

/* apis */
control_t     *make_control( void );
void           init_control( control_t *control, closure_t *module, control_t *caller, size_t n );
void           free_control( control_t *control );

control_t     *make_call_frame( control_t *caller, closure_t *module, size_t n );
void           free_call_frame( control_t **returner );

environment_t *make_environment( void );
void           init_environment( environment_t *environment, bool is_lenvt, bool is_captured, environment_t *next, value_t *ini, size_t n );
void           free_environment( environment_t *environment );

value_t        envt_ref( environment_t *environment, size_t i, size_t j );
value_t        envt_set( environment_t *environment, size_t i, size_t j, value_t value );
void           envt_bind( environment_t *environment, value_t *buffer, size_t n );
void           envt_unbind( environment_t *environment, size_t n );
void           envt_capture( environment_t *environment );

/* utilities/convenience */
value_t  *rl_push( value_t x );
value_t   rl_pop( void );
value_t  *rl_pushn( size_t n );
value_t   rl_popn( size_t n );
value_t   rl_peek( size_t n );

void      rl_panic( const char *fmt, ... );
bool      rl_recover( void );

/* initialization */
void      rl_rlvm_init( void );

#endif
