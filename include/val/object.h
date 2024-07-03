#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* Common Obj type and API functions defined here */

/* C types*/
// common object header macro
#define HEADER                                   \
  Obj*    next;                                  \
  Proc*   ownp;                                 \
  Type*   type;                                  \
  Map*    meta;                                  \
  word_t  hash  : 48;                            \
  word_t  ownh  :  1;                            \
  word_t  trace :  1;                            \
  word_t  free  :  1;                            \
  word_t  sweep :  1;                            \
  word_t  gray  :  1;                            \
  word_t  black :  1

struct Obj {
  HEADER;

  // bit fields
  word_t flags : 10;

  // data fields
  byte data[];
};

/* Globals */
extern MMap MetaData; // MetaData objects for immediate values

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
void*  new_obj(Proc* p, Type* t);
void*  clone_obj(void* x);
void   init_obj(Proc* p, Type* t, Obj* o);
void   free_obj(void* x);
size_t sweep_obj(void* x);

// metadata generics
#define meta(x)                                 \
  generic((x),                                  \
          Val:val_meta,                         \
          default:obj_meta)(x)

#define get_meta(x, k)                              \
  generic((x),                                      \
          Val:generic((k),                          \
                      char*:vs_get_meta,            \
                      Val:vv_get_meta),             \
          default:generic((k),                      \
                          char*:os_get_meta,        \
                          Val:ov_get_meta))(x, k)

#define set_meta(x, k, v)                           \
  generic((x),                                      \
          Val:generic((k),                          \
                      char*:vs_set_meta,            \
                      Val:vv_set_meta),             \
          default:generic((k),                      \
                       char*:os_set_meta,           \
                       Val:ov_set_meta))(x, k, v)

#define meta_eq(x, k, v)                            \
  generic((x),                                      \
          Val:generic((k),                          \
                      char*:vs_meta_eq,             \
                      Val:vv_meta_eq),              \
          default:generic((k),                      \
                          char*:os_meta_eq,         \
                          Val:ov_meta_eq))(x, k, v)

// metadata methods
Map* val_meta(Val x);
Map* obj_meta(void* x);

Val vs_get_meta(Val x, char* k);
Val vv_get_meta(Val x, Val k);
Val os_get_meta(void* x, char* k);
Val ov_get_meta(void* x, Val k);

void vs_set_meta(Val x, char* k, Val v);
void vv_set_meta(Val x, Val k, Val v);
void os_set_meta(void* x, char* k, Val v);
void ov_set_meta(void* x, Val k, Val v);

bool vs_meta_eq(Val x, char* k, Val v);
bool vv_meta_eq(Val x, Val k, Val v);
bool os_meta_eq(void* x, char* k, Val v);
bool ov_meta_eq(void* x, Val k, Val v);

#endif
