#ifndef rl_val_h
#define rl_val_h

#include "rascal.h"

/* C types */
#include "tpl/decl/alist.h"
ALIST(vals, val_t);

typedef union rl_data_t rl_data_t;
typedef enum val_type_t val_type_t;

union rl_data_t {
  val_t    as_val;
  real_t   as_real;
  small_t  as_small;
  obj_t    as_obj;
  sym_t    as_sym;
  native_t as_native;
  prim_t   as_prim;
  module_t as_module;
  vec_t    as_vec;
  cons_t   as_cons;
  code_t   as_code;
};

enum val_type_t {
  real_val,
  small_val,
  obj_val
};

/* globals */
#define QNAN    0x7ff8000000000000ul
#define RNAN    (nan(""))

#define REAL    0x0000000000000000ul
#define SMALL   0x7ffc000000000000ul
#define OBJECT  0x7fff000000000000ul

#define TAGMASK 0xffff000000000000ul
#define PTRMASK (~TAGMASK)

/* APIs */
val_type_t val_type(val_t val);
type_t     val_type_of(val_t val);
bool       val_has_type(val_t val, type_t type);
bool       is_val_type(val_t val, val_type_t valtype);

/* convenience */
#define tag_of(x)     ((x)&TAGMASK)
#define data_of(x)    ((x)&PTRMASK)
#define tag_val(x, t) ((as_val(x)&PTRMASK)|(t))

#define as_val(x)     (((rl_data_t)(x)).as_val)
#define as_obj(x)                                   \
  _Generic((x),                                     \
           val_t:((obj_t)(((val_t)(x))&PTRMASK)),   \
           default:((obj_t)((typeof(x))(x))))

#define is_obj(x)     is_val_type(x, obj_val)
#define is_nul(x)     ((x)==OBJECT)

#endif
