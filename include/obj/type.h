#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "repr.h"

#include "obj/object.h"

/* commentary */

/* C types */
struct type_t
{
  OBJHEAD;
  
  /* misc */
  char        *name;
  uint64_t     idno;

  /* size/layout/representation information */
  size_t       ob_size;
  repr_t       ob_repr;
  bool         is_leaf;
  value_t      val_tag;

  /* additional array layout information */

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
