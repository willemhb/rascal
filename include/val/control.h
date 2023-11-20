#ifndef rl_val_control_h
#define rl_val_control_h

#include "vm/context.h"

#include "val/object.h"

/* Object APIs and declarations for the types used to represent reified continuations. */

/* C types */
/* Represents a reified continuation. */
struct Control {
  HEADER;
  Closure*   code;
  Chunk*     effh;
  uint16_t*  ip;
  size_t     bp;
  size_t     cp;
  size_t     n_frames, n_vals;
  ExecFrame* frames;
  Value*     values;
};

/* Globals */
extern Type ControlType;

/* External API */
#define is_cntl(x) has_type(x, &ControlType)
#define as_cntl(x) as(Control*, untag48, x)

Control* mk_cntl(Closure* code, Chunk* effh, uint16_t* ip, size_t nf, size_t nv, ExecFrame* frames, Value* vals);

#endif
