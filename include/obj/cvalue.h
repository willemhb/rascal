#ifndef rascal_cvalue_h
#define rascal_cvalue_h

#include "obj/boxed.h"

// function pointer types
typedef size_t     (*cvsize_t)( cvalue_t *cvalue );
typedef size_t     (*cvunbox_t)( cvalue_t *cvalue, size_t size, void *spc );
typedef rl_value_t (*cvunwrap_t)( cvalue_t *cv );

struct cvalue_t
{
  BOX

  byte     space[0];
};

struct cv_impl_t
{
  bool       bytes;     // if true, value is a heterogenous struct (representing a single value)
  bool       fits_word;
  bool       fits_fix;
  bool       fits_imm;

  cvsize_t   cvsize;
  cvunbox_t  cvunbox;
  cvunwrap_t cvunwrap;
};

// forward declarations
size_t     cv_unbox( cvalue_t *cv, size_t size, void *buf );
size_t     cv_size( cvalue_t *cv );
rl_value_t cv_unwrap( cvalue_t *cv );

// convenience
#define is_cvalue(x) is_repr(x, CVALUE)
#define as_cvalue(x) ((cvalue_t*)as_ptr(x))

#define CVAL cvalue_t cval;

#endif
