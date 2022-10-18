#ifndef rascal_type_h
#define rascal_type_h

#include "obj/boxed.h"

// function pointers
typedef bool       (*isa_t)( type_t *self, value_t val );
typedef bool       (*has_t)( type_t *self, value_t other );
typedef value_t    (*box_t)( type_t *self, rl_value_t val );
typedef size_t     (*unbox_t)( type_t *self, rl_value_t val, size_t size, void *spc );

typedef struct dtype_impl_t dtype_impl_t;
typedef struct utype_impl_t utype_impl_t;
typedef struct class_impl_t class_impl_t;

typedef struct ar_impl_t ar_impl_t;
typedef struct tb_impl_t tb_impl_t;
typedef struct ht_impl_t ht_impl_t;
typedef struct at_impl_t at_impl_t;
typedef struct cv_impl_t cv_impl_t;
typedef struct ob_impl_t ob_impl_t;

struct type_t
{
  BOX

  symbol_t   *name;
  function_t *constructor;
  isa_t       isa;
  has_t       has;

  hash_t      hash;

  bool        is_ptype; // builtin type?
  bool        is_dtype; // concrete type?
  bool        is_utype; // union type?
  bool        is_class; // type class?

  union
  {
    dtype_impl_t *dtype;
    utype_impl_t *utype;
    class_impl_t *class;
  } impl;
};

struct utype_impl_t
{
  table_t *members;
};

struct class_impl_t
{
  table_t *members;     // implementing types
  table_t *supers;      // classes extended
  table_t *subs;        // classes extending
  table_t *signatures;  // method signatures
};

struct dtype_impl_t
{
  size_t   base_size;    // base size of the represented data (including header, if object)
  Ctype_t  Ctype;        // Ctype of the represented data
  repr_t   repr;         // value type of the represented data
  value_t  instance;     // singletons and empty instances (for collection types)

  // boxing and unboxing
  box_t     box;
  unbox_t   unbox;
  obfree_t  obfree;
  obtrace_t obtrace;

  // further implementation methods
  union
  {
    ar_impl_t *arr;    // array types
    tb_impl_t *tab;    // table types
    ht_impl_t *hamt;   // hamt types
    at_impl_t *amt;    // amt types
    cv_impl_t *cval;   // cvalue types
    ob_impl_t *obj;    // other object types (mostly internals)
  } impl;
};

// globals
extern type_t *ReprTypes[N_REPR];

// convenience
#define as_type(x) ((type_t*)as_ptr(x))
#define is_type(x) is_repr(x, TYPE)

#endif
