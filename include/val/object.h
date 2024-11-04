#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* Common Obj type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                       \
  Obj*    heap;                      \
  Map*    meta;                      \
  hash_t  hash;                      \
  Type    tag;                       \
  union {                            \
    uint8 mflags;                    \
                                     \
    struct {                         \
      uint8 trace      : 1;          \
      uint8 sweep      : 1;          \
      uint8 free       : 1;          \
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
size64 sweep_obj(void* x);

#endif
