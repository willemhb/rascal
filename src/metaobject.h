#ifndef metaobject_h
#define metaobject_h

#include "value.h"

#define TYPE_HEADER type_t type

// C types --------------------------------------------------------------------
struct type_t {
  HEADER;
  char*    name;
  uint64   idno;
  kind_t (*isa)(value_t v, type_t* self);
};

struct data_type_t {
  TYPE_HEADER;
  usize     size;
  dict_t*   slots;
  object_t* member; // for unique empty instance of immutable type ------------

  // vtable -------------------------------------------------------------------
  // sacred methods -----------------------------------------------------------
  void  (*print)(value_t val, port_t* ios);
  usize (*size_of)(void* ptr);
  uhash (*hash)(void* ptr);
  bool  (*equal)(void* x, void* y);
  int   (*compare)(value_t x, value_t y);

  // lifetime methods ---------------------------------------------------------
  void  (*init)(void* self);
  void  (*trace)(void* self);
  void  (*free)(void* self);

  // misc ---------------------------------------------------------------------
  int   (*write)(type_t* type, value_t val, void* spc);
};

struct union_type_t {
  TYPE_HEADER;
  dict_t* members;
};

// API ------------------------------------------------------------------------

#endif
