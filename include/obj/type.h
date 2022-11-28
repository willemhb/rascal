#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "vmtype.h"

#include "obj/object.h"

/* commentary */

/* C types */
struct type_t
{
  ulong idno;
  ulong hash;
  struct object_t obj;
  char name[];
};

struct uniontype_t
{
  struct uniontype_t *next;
  struct type_t      *member;
  struct type_t type;
};

struct datatype_t
{
  
  uint     flags;
  vmtype_t vmtype;
  
  struct type_t type;
};

/* globals */
extern struct type_t TypeType;

/* API */
size_t type_base_size( type_t type );

/* runtime */
void rl_obj_type_init( void );

/* convenience */
#define is_type( x )     (rl_typeof(x)==&TypeType.data)
#define as_type( x )     ((type_t)((x)&PTRMASK))

#endif
