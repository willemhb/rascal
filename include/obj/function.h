#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "def/opcodes.h"

#include "vm/obj.h"

#include "vm/obj/support/vector.h"

/* commentary */

/* C types */

/* a builtin VM function */
struct prim_t {
  OBJ;

  opcode_t label;
};

/* a builtin C function */
struct native_t {
  OBJ;

  val_t (*callback)(val_t *args, size_t n);
};

/* a reified continuation */
struct cntl_t {
  OBJ;

  vector_t   stack;      /* slice of the stack between prompt and TOS */
  module_t  *module;     /* the module executing when the control object was created */
  closure_t *function;   /* the function executing when the control object was created */
  size_t bp, ip, cp;     /* self explanatory (pp discarded) */
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
