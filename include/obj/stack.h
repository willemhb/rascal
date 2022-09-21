#ifndef rascal_stack_h
#define rascal_stack_h

#include "obj.h"
#include "template/array.h"

typedef struct stack_t stack_t;

ARRAYLIST(stack, val_t);

// forward declarations -------------------------------------------------------
arity_t stack_push( obj_t *stk, val_t val );
val_t   stack_pop( obj_t *stk );

// toplevel dispatch ----------------------------------------------------------
void stack_mark( void );
void stack_init( void );

#endif
