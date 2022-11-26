#ifndef rl_obj_bytecode_h
#define rl_obj_bytecode_h

#include "obj/array.h"
#include "tpl/decl/array.h"

/* commentary */

/* C types */
struct bytecode_t
{
  struct array_t arr;

  ushort code[];
};

/* globals */
extern struct type_t BytecodeType;

/* API */
bytecode_t make_bytecode( size_t n, ushort *ini );
void free_bytecode( bytecode_t bytecode );

ushort bytecode_get( bytecode_t bytecode, long i );
ushort bytecode_set( bytecode_t bytecode, long i,  ushort x );
size_t bytecode_add( bytecode_t bytecode, size_t n, ... );

/* runtime */
void rl_obj_bytecode_init( void );

/* convenience */


#endif
