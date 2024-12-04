#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

#include "vm/type.h"

/* Common Obj type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                          \
  Obj*    heap;                         \
  Table*  meta;                         \
  union {                               \
    struct {                            \
      word_t hash    : 48;              \
      word_t tag     :  8;              \
      word_t _flag   :  1;              \
      word_t nohash  :  1;              \
      word_t notrace :  1;              \
      word_t nosweep :  1;              \
      word_t nofree  :  1;              \
      word_t sealed  :  1;              \
      word_t black   :  1;              \
      word_t gray    :  1;              \
    };                                  \
    struct {                            \
    word_t : 56;                        \
      word_t mflags  :  8;              \
    };                                  \
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
void* clone_obj(State* vm, void* x);
void* seal_obj(State* vm, void* x);   // permanently mark an object as read-only
void* unseal_obj(State* vm, void* x); // return the unsealed object or an editable copy

#endif
