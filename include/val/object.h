#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

#include "vm/type.h"

/* Common Obj type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                       \
  Obj*    heap;                      \
  Table*  meta;                      \
  hash_t  hash;                      \
  Type    tag;                       \
  union {                            \
    uint8 mflags;                    \
                                     \
    struct {                         \
      uint8 notrace    : 1;          \
      uint8 nosweep    : 1;          \
      uint8 nofree     : 1;          \
      uint8 persistent : 1;          \
      uint8 transient  : 1;          \
      uint8 sealed     : 1;          \
      uint8 black      : 1;          \
      uint8 gray       : 1;          \
    };                               \
  }

struct Obj {
  HEADER;

  // data fields
  byte data[];
};

/* External APIs */
// lifetime APIs
#define mark(vm, x)  generic2(mark, x, vm, x)
#define trace(vm, x) generic2(trace, x, vm, x)

// mark methods
void val_mark(State* vm, Val x);
void obj_mark(State* vm, void* x);

// trace method
void val_trace(State* vm, Val x);
void obj_trace(State* vm, void* x);

// other lifetime methods
void* new_obj(State* vm, Type t, flags32 f);
void  init_obj(State* vm, Obj* o, Type t, flags32 f);
void  free_obj(State* vm, void* x);
void  sweep_obj(State* vm, void* x);

#endif
