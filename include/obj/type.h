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
  bool         is_leaf;     // doesn't contain a pointer to another object
  value_t      val_tag;
  value_t      constructor; // special constructor (if one exists)

  /* internal methods */
  make_fn_t     make_fn;
  init_obj_fn_t init_fn;
  init_spc_fn_t cvinit_fn;
  free_fn_t     free_fn;
  trace_fn_t    trace_fn;
  compare_fn_t  compare_fn;
  hash_fn_t     hash_fn;
};

/* globals */
/* builtin type idnos */
enum
  {
    nul_idno, bool_idno, smint_idno, fixnum_idno, real_idno,

    type_idno, atom_idno, cons_idno,

  } type_order;

extern type_t TypeType;

/* API */

/* runtime */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );

/* convenience */
static inline bool    is_type( value_t x ) { return rl_isa(x, &TypeType); }
static inline type_t *as_type( value_t x ) { return (type_t*)as_object(x); }

#endif
