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

ushort bytecode_get( bytecode_t *bytecode, long i );
ushort bytecode_set( bytecode_t *bytecode, long i );
size_t bytecode_add( bytecode_t *bytecode, size_t n, ... );

/* runtime */
void rl_obj_bytecode_init( void );
void rl_obj_bytecode_mark( void );

/* convenience */
static inline bytecode_t *as_bytecode( value_t x ) { return (bytecode_t*)as_object(x); }
static inline bool        is_bytecode( value_t x ) { return rl_isa(x, &BytecodeType); }

#endif
