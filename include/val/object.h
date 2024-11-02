#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* Common Obj type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                                    \
  Obj*     heap;                                  \
  flags64  hash  : 48;                            \
  flags64  mut   :  1;                            \
  flags64  trans :  1;                            \
  flags64  ownh  :  1;                            \
  flags64  trace :  1;                            \
  flags64  free  :  1;                            \
  flags64  sweep :  1;                            \
  flags64  gray  :  1;                            \
  flags64  black :  1;                            \
  flags64  tag   :  5

struct Obj {
  HEADER;

  // data fields
  byte data[];
};

/* APIs */
// lifetime APIs
#define mark(x)                                 \
  generic((x),                                  \
          Val:mark_val,                         \
          default:mark_obj)(x)

// mark methods
void mark_val(Val v);
void mark_obj(void* x);

// trace method
void trace(void* x);

// other lifetime methods
void*  new_obj(Type t);
void*  clone_obj(void* x);
void   init_obj(Type t, Obj* o);
void   free_obj(void* x);
size_t sweep_obj(void* x);

#endif
