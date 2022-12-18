#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "def/opcodes.h"

#include "vm/obj.h"

/* commentary */

/* C types */
struct prim_t {
  OBJ;

  opcode_t label;
};

struct native_t {
  OBJ;

  val_t (*callback)(val_t *args, size_t n);
};

struct cntl_t {
  OBJ;

  
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
