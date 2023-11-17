#ifndef rl_val_control_h
#define rl_val_control_h

#include "vm/context.h"

#include "val/object.h"

/* Object APIs and declarations for the types used to represent reified continuations. */

/* C types */
/* Represents a reified continuation. */
struct Control {
  HEADER;
  size_t     n_frames, n_vals;
  ExecFrame* frames;
  Value*     values;
};

/* Globals */
extern Type ControlType;

/* External API */


#endif
