#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* Common Object type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                                   \
  Object* next;                                  \
  Type*   type;                                  \
  Map*    meta;                                  \
  word_t  hash  : 48;                            \
  word_t  trace :  1;                            \
  word_t  free  :  1;                            \
  word_t  sweep :  1;                            \
  word_t  gray  :  1;                            \
  word_t  black :  1

struct Object {
  HEADER;

  // bit fields
  word_t flags : 11;

  // data fields
  byte_t data[];
};

/* APIs */
// lifetime APIs
#define mark(x)                                 \
  generic((x),                                  \
          Value:mark_val,                       \
          default:mark_obj)(x)

// mark methods
void mark_val(Value v);
void mark_obj(void* x);

// trace method
void trace(void* x);

// other lifetime methods
void* new_obj(Type* t);
void  init_obj(Type* t, Object* o);
void  free_obj(void* x);
void  sweep_obj(void* x);

#endif
