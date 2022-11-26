#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "obj/bytecode.h"
#include "obj/type.h"

#include "tpl/impl/array.h"

/* C types */

/* globals */

struct type_t BytecodeType =
  {
    { .type=&TypeType.data, .size=sizeof(struct type_t) },

    {
      .name="bytecode",
      .vmtype=vmtype_array,
      .obsize=sizeof(struct bytecode_t),
      .elsize=sizeof(ushort),
      .stringp=false
    }
  };

/* API */
MAKE_ARR(BytecodeType.data, bytecode, ushort);
FREE_ARR(bytecode, ushort);
ARRAY_REF(bytecode, ushort);
ARRAY_SET(bytecode, ushort);
ARRAY_ADD(bytecode, ushort, uint);

/* runtime */
void rl_obj_bytecode_init( void ) {}

/* convenience */
