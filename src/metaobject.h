#ifndef metaobject_h
#define metaobject_h

#include "value.h"

#define TYPE_HEADER rl_type_t type

// C types --------------------------------------------------------------------
struct rl_type_t {
  HEADER;
  char*  name;
  uint64 idno;
  kind_t (*isa)(value_t v, rl_type_t* self);
  vtable_t* vtable;
};

struct vtable_t {
  void    (*print)(value_t x, port_t* ios);
  usize   (*size)(object_t* o);
  uhash   (*hash)(object_t* o);
  bool    (*equal)(object_t* x, object_t* y);
  int     (*compare)(value_t x, value_t y);
};

struct primitive_type_t {
  TYPE_HEADER;
  usize size;
  int (*init)(void* spc, value_t val);
};

struct object_type_t {
  TYPE_HEADER;
  usize size;
  int   (*init)(object_t* self, void* ini);
  void  (*trace)(object_t* self);
  void  (*free)(object_t* self);
  dict_t* slots;
  object_t* singleton; // for unique empty instance of immutable type ---------
};

struct union_type_t {
  TYPE_HEADER;
  set_t* members;
};

// API ------------------------------------------------------------------------

#endif
