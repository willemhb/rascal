#ifndef rl_obj_buffer_h
#define rl_obj_buffer_h

#include "obj/array.h"
#include "tpl/decl/array.h"

/* commentary */

/* C types */
struct buffer_t
{
  struct array_t arr;
  char data[];
};

/* globals */
extern struct type_t BufferType;

/* API */
MAKE_ARR(buffer, char);
FREE_ARR(buffer, char);
RESIZE_ARR(buffer, char);
ARRAY_REF(buffer, char);
ARRAY_SET(buffer, char);
ARRAY_ADD(buffer, char);
ARRAY_POP(buffer, char);

/* runtime */
void rl_obj_buffer_init( void );

/* convenience */
#define is_buffer( x )     (rl_typeof(x)==&BufferType.data)
#define as_buffer( x )     ((buffer_t)((x)&PMASK))

#define buffer_header( x ) ((struct buffer_t*)array_header((array_t)(x)))
#define buffer_data( x )   ((buffer_t)array_data((array_t)(x)))
#define buffer_len( x )    (buffer_header(x)->arr.len)
#define buffer_alloc( x )  (buffer_header(x)->arr.alloc)

#endif
