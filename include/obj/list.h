#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj/object.h"
#include "obj/type.h"

struct pair_t
{
  object_t object;
  
};

struct cons_t
{
  object_t object;

  arity_t  length;
  bool     proper;

  value_t  car;

  union
  {
    value_t cdr;
    cons_t *tail;
  };
};

// globals
extern type_t *ConsType, *NulType;

// api
cons_t *new_cons( void );

// convenience
#define is_nul(val)    ((val)==NUL_VAL)
#define is_cons(val)   is_repr(val, CONS)
#define as_cons(val)   ((cons_t*)as_ptr(val))

// gc and initialization
void cons_init( void );
void cons_roots( void );

static inline void cons_types_init( void )
{
  extern dtype_t    ConsTypeOb, NulTypeOb;
  extern function_t ConsFunc, NulFunc;

  ConsType = &ConsTypeOb.type; ConsType->constructor = &ConsFunc;
  NulType  = &NulTypeOb.type;  NulType->constructor  = &NulFunc;
}


#endif
