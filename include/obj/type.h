#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "obj/object.h"

/* commentary */

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
  size_t       ob_size;     // 
  size_t       el_size;     // base size of the object pointed to by the value

  /* misc */
  char        *name;

  /* internal methods */
  make_fn_t    make_fn;
  init_fn_t    init_fn;
  free_fn_t    free_fn;
  trace_fn_t   trace_fn;
  compare_fn_t compare_fn;
  hash_fn_t    hash_fn;
};

/* globals */
extern type_t TypeType;

/* API */

/* runtime */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );

/* convenience */
static inline bool    is_type( value_t x ) { return rl_isa(x, &TypeType); }
static inline type_t *as_type( value_t x ) { return (type_t*)as_object(x); }

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

#define TYPEINIT(Ctype, name, fields, n_fields)				\
  {									\
    NULL,								\
      NUL,								\
      &TypeType,							\
      sizeof(type_t)+sizeof(field_t)*(n_fields+8)+sizeof(name),		\
      true,								\
      false,								\
      false,								\
      0									\
      },								\
    fields,								\
    n_fields+8,								\
    sizeof(Ctype),							\
    0									\

#endif
