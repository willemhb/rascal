#ifndef rascal_obj_control_h
#define rascal_obj_control_h

#include "obj/object.h"

/* C types */
struct control_t
{
  OBJHEAD;

  control_t     *next;        // continuation
  environment_t *environment; // current environment
  lambda_t      *function;    // executing function
  ushort        *ip;          // program counter
};

/* globals */
extern type_t ControlType;

/* runtime */
void rl_obj_control_init( void );
void rl_obj_control_mark( void );

/* API */

/* convenience */
static inline bool       is_control( value_t x ) { return rl_isa(x, &ControlType); }
static inline control_t *as_control( value_t x ) { return (control_t*)as_object(x); }

#endif
