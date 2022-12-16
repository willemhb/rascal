#ifndef rl_obj_code_h
#define rl_obj_code_h

#include "rascal.h"

#include "def/opcodes.h"

#include "vm/object.h"

/* commentary

   Holds compiled (or compiling) programs. */

/* C types */
struct rl_code_t
{
  rl_object_t  obj;

  vector_t     constants;
  bytecode_t   instructions;
};

/* globals */
extern rl_datatype_t CodeType;

/* API */
/* constructors */
rl_code_t *make_code( vector_t constants, bytecode_t instructions );

/* interfaces */
/* constant store interface */
rl_value_t get_code_constant( rl_code_t *code, size_t n );
bytecode_t get_code_instructions( rl_code_t *code );

/* runtime dispatch */
void rl_obj_code_init( void );
void rl_obj_code_mark( void );
void rl_obj_code_cleanup( void );

/* convenience */

#endif
