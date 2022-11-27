#include "obj/buffer.h"
#include "obj/type.h"

#include "tpl/impl/array.h"

/* commentary */

/* C types */

/* globals */
struct type_t BufferType =
  {
   {
    .type=&TypeType.data,
    .size=sizeof(struct type_t)
   },
   {
    .name="buffer",
    .vmtype=vmtype_array,
    .obsize=sizeof(struct buffer_t),
    .elsize=sizeof(char),
    .stringp=true
   }
  };

/* API */
MAKE_ARR(BufferType.data, buffer, char);
FREE_ARR(buffer, char);
RESIZE_ARR(buffer, char);
ARRAY_REF(buffer, char);
ARRAY_SET(buffer, char);
ARRAY_ADD(buffer, char, uint);
ARRAY_POP(buffer, char);

/* runtime */
void rl_obj_buffer_init( void ) {}

/* convenience */
