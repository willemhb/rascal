#ifndef rl_obj_bytecode_h
#define rl_obj_bytecode_h

#include "obj/object.h"
#include "tpl/decl/array.h"

/* C types */
struct bytecode_t { ARRAY(ushort); };

/* globals */
extern type_t BytecodeType;

/* API */
void resize_bytecode( bytecode_t *bytecode, size_t new_size );

/* initialization */
void rl_obj_bytecode_init( void );

/* convenience & utilities */
#define as_bytecode( x ) ((bytecode_t*)as_obj(x))
#define is_bytecode( x ) value_is_type(x, &BytecodeType)

#endif
