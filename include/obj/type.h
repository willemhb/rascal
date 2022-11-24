#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "obj/object.h"

/* C types */
typedef struct field_t
{
  uint   repr     :  6;
  uint   read     :  1;
  uint   write    :  1;
  uint            : 24;
  uint   offset;
} field_t;

struct type_t
{
  OBJHEAD;
  /* size/layout information */
  field_t     *fields;      // description of object layout (or NULL)
  size_t       n_fields;    // total number of distinct fields on the object
  size_t       ob_size;     // base size of the object pointed to by the value

  /* misc */
  string_t    *name;

  /* internal methods */
  init_fn_t    init_fn;
  compare_fn_t compare_fn;
  hash_fn_t    hash_fn;
};

/* globals */
extern type_t TypeType;

/* convenience & utilities */
#define as_type( x ) ((type_t*)as_obj(x))
#define is_type( x ) value_is_type(x, &TypeType)

// initial object layout (for initializing type->fields)
#define OBJLAYOUT							\
  { repr_obj, false, false, offsetof(object_t, next) },			\
  { repr_val, false, false, offsetof(object_t, _meta) },		\
  { repr_obj, false, false, offsetof(object_t, type) },			\
  { repr_uint32, false, false, offsetof(object_t, size) },		\
  { repr_uint8,  false, false, offsetof(object_t, gray) },		\
  { repr_uint8,  false, false, offsetof(object_t, black) },		\
  { repr_uint8,  false, false, offsetof(object_t, allocated) },		\
  { repr_uint8,  false, false, offsetof(object_t, flags) }


#endif
