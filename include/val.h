#ifndef rl_val_h
#define rl_val_h

#include "rascal.h"

/* C types */
#include "tpl/decl/alist.h"
ALIST(vals, val_t);

typedef union rl_data_t rl_data_t;

union rl_data_t {
  val_t   as_val;
  real_t  as_real;
  obj_t   as_obj;
  sym_t  *as_sym;
  cons_t *as_cons;
};

/* globals */
#define QNAN    0x7ff8000000000000ul
#define RNAN    (nan(""))

#define REAL    0x0000000000000000ul
#define OBJECT  0x7fff000000000000ul

#define TAGMASK 0xffff000000000000ul
#define PTRMASK (~TAGMASK)

/* API */
val_type_t val_type(val_t x);
rl_type_t  rl_type(val_t x);
bool       has_val_type(val_t x, val_type_t type);
bool       has_rl_type(val_t x, rl_type_t type);

/* initialization */
void val_init( void );

/* convenience */
#define tag_of(x)     ((x)&TAGMASK)
#define data_of(x)    ((x)&PTRMASK)
#define tag_val(x, t) ((as_val(x)&PTRMASK)|(t))

#define as_val(x)     (((rl_data_t)(x)).as_val)
#define as_real(x)    (((rl_data_t)(x)).as_real)
#define as_obj(x)     ((obj_t)((x)&PTRMASK))

#define is_real(x)    has_val_type(x, real_val)
#define is_obj(x)     has_val_type(x, obj_val)
#define is_nul(x)     ((x)==OBJECT)

#endif
