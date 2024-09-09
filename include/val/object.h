#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* Common Obj type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                                   \
  Obj*    heap;                                  \
  word_t  hash  : 48;                            \
  word_t  type  : 10;                            \
  word_t  ownh  :  1;                            \
  word_t  trace :  1;                            \
  word_t  free  :  1;                            \
  word_t  sweep :  1;                            \
  word_t  gray  :  1;                            \
  word_t  black :  1

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
