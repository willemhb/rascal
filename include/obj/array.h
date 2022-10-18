#ifndef rascal_array_h
#define rascal_array_h

#include "obj/type.h"

// function pointer types
typedef size_t     (*alength_t)( array_t *array );                      // abstract arity
typedef size_t     (*asize_t)( array_t *array );                        // allocated size
typedef void       (*aresize_t)( array_t *array, size_t newl );
typedef rl_value_t (*aref_t)(array_t *array, size_t n );
typedef rl_value_t (*aset_t)(array_t *array, size_t n, rl_value_t x );
typedef size_t     (*aput_t)(array_t *array, size_t n, rl_value_t x );
typedef rl_value_t (*apop_t)(array_t *array, size_t n );

// generic array type (all values of type array can be safely cast to this type)
struct array_t
{
  BOX

  void   *data;
  size_t  size[1];
};

struct ar_impl_t
{
  type_t   *eltype;
  ushort    elsize;   // element size
  byte      elCtype;  // element Ctype
  byte      unboxed;  // elements unboxed?
  byte      multidim; // 1-dimensional?
  byte      encoded;  // string?
  byte      mutable;  // setable?
  byte      dynamic;  // resizable?

  asize_t   asize;
  alength_t alength;
  aresize_t aresize;
  aref_t    aref;
  aset_t    aset;
  aput_t    aput;
  apop_t    apop;

  size_t    size[];   // exact interpretation depends on flags above
};

// convenience
#define as_array(x) ((array_t*)as_ptr(x))

#endif
