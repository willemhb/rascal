#ifndef rl_function_h
#define rl_function_h

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */
struct Closure {
  HEADER;

  // data fields
  Binary*  code;
  Vector*  vals;
  Environ* envt;
};

struct Native {
  HEADER;

  rl_native_fn_t C_function;
};

struct Generic {
  HEADER;

  // data fields
  MutMap* cache;
  MTRoot* root;
};

// control type (reified continuation)
typedef struct IFrame IFrame;

struct Control {
  HEADER;

  // data fields
  IFrame* frames;   // preserved stack frames
  size_t  fcount;   // frame count
  MutVec* sbuffer;  // buffer for stack values
};

/* Globals */
extern Type ClosureType, NativeType, GenericType, ControlType, MTRootType, MTNodeType, MTLeafType;

/* APIs */

#endif
