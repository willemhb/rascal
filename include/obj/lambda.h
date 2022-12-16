#ifndef rl_obj_lambda_h
#define rl_obj_lambda_h

#include "def/opcodes.h"

#include "vm/object.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/vector.h"

/* commentary

   Representation of a user function. */

/* C types */
struct lambda_t
{
  object_t obj;

  size_t     nargs;
  symbol_t  *name;
  rl_code_t *code;
};

/* globals */
extern datatype_t LambdaType;

/* API */
/* constructors */
lambda_t *make_lambda( size_t nargs, symbol_t *name, rl_code_t *code );

/* accessors */
/* miscellaneous accessors */
symbol_t  *get_lambda_name( lambda_t *lambda );
size_t     get_lambda_nargs( lambda_t *lambda );
rl_code_t *get_lambda_code( lambda_t *lambda );

/* runtime dispatch */
void rl_obj_lambda_init( void );
void rl_obj_lambda_mark( void );
void rl_obj_lambda_cleanup( void );

/* convenience */
#define is_lambda( x )   (rl_typeof(x)==&LambdaType)
#define as_lambda( x )   ((lambda_t*)((x)&PTRMASK))

#endif
