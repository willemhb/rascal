#ifndef rl_obj_lambda_h
#define rl_obj_lambda_h

#include "def/opcodes.h"

#include "vm/object.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/vector.h"

/* commentary

   compiled code representation. */

/* C types */
struct lambda_t
{
  object_t obj;

  symbol_t    *name;
  size_t       nargs;
  namespc_t *namespc;
  vector_t     constants;
  bytecode_t   instructions;
};

/* globals */
extern datatype_t LambdaType;

/* API */
/* constructors */
lambda_t *make_lambda( symbol_t *name, namespc_t *parent_ns );

/* accessors */
/* miscellaneous accessors */
symbol_t   *get_lambda_name( lambda_t *lambda );
size_t      get_lambda_nargs( lambda_t *lambda );

/* namespc interface */
int lookup_in_lambda_namespc( lambda_t *lambda, symbol_t *name, size_t *i, value_t *bind );
int define_in_lambda_vars( lambda_t *lambda, symbol_t *name );
int define_in_lambda_macros( lambda_t *lambda, symbol_t *name );

/* constant store interface */
value_t   get_lambda_const( lambda_t *lambda, size_t n );
size_t    put_lambda_const( lambda_t *lambda, value_t x );
void      finalize_lambda_const( lambda_t *lambda );

/* instructions interface */
size_t    get_instr_len( lambda_t *lambda );
size_t    emit_lambda_instr( lambda_t *lambda, opcode_t op, ... );
size_t    fill_lambda_instr( lambda_t *lambda, int offset, int arg );
void      finalize_lambda_instr( lambda_t *lambda );

/* runtime */
void rl_obj_lambda_init( void );
void rl_obj_lambda_mark( void );
void rl_obj_lambda_cleanup( void );

/* convenience */
#define is_lambda( x )   (rl_typeof(x)==&LambdaType)
#define as_lambda( x )   ((lambda_t*)((x)&PTRMASK))

#endif
