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

/* initialization */
void rl_control_init( void );

/* API */

/* convenience & utilities */
#define is_control( x )      value_is_type(x, &ControlType)
#define as_control( x )     ((control_t*)as_obj(x))

#endif
