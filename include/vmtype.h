#ifndef rl_vmtype_h
#define rl_vmtype_h

#include "common.h"

/* commentary

   Various definitions and globals that provide a low-level
   description of rascal fields & values. */

/* C types */
typedef enum vmtype_t vmtype_t;
typedef enum numtype_t numtype_t;
typedef enum objtype_t objtype_t;
typedef struct vmtype_spec_t vmtype_spec_t;

enum vmtype_t
  {
    vmtype_flo64,
    vmtype_stream,
    vmtype_object,
    vmtype_array
  };

enum numtype_t
  {
    numtype_void,
    numtype_signed,
    numtype_unsigned,
    numtype_float
  };

struct vmtype_spec_t
{
  char     *name;
  size_t    size;
  size_t    align;
  numtype_t numtype;
  uint      flags;
};

/* globals */
extern vmtype_spec_t TypeSpecs[];

/* API */

/* convenience */

#endif
