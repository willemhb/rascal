#ifndef rl_object_h
#define rl_object_h

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
rl_status_t mark_val(Value v);
rl_status_t mark_obj(void* o);

rl_status_t trace(void* o);

#endif
