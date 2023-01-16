#ifndef rl_val_h
#define rl_val_h

#include "rascal.h"

/* C types */
#include "decl/alist.h"
ALIST(Values, Value);

typedef union rl_data_t rl_data_t;
typedef enum val_type_t val_type_t;

union rl_data_t {
  val_t     as_val;
  real_t    as_real;
  small_t   as_small;
  bool_t    as_bool;
  glyph_t   as_glyph;
  native_t  as_native;
  obj_t     as_obj;

  sym_t     as_sym;
  func_t    as_func;
  cons_t    as_cons;
  vec_t     as_vec;
  str_t     as_str;

  upval_t   as_upval;
  namespc_t as_namespc;
  module_t  as_module;
  method_t  as_method;
  code_t    as_code;
};

enum val_type_t {
  real_val,
  small_val,
  bool_val,
  glyph_val,
  native_val,
  obj_val
};

/* globals */
#define QNAN    0x7ff8000000000000ul
#define RNAN    (nan(""))

#define REAL    0x0000000000000000ul
#define SMALL   0x7ffc000000000000ul
#define BOOL    0x7ffd000000000000ul
#define GLYPH   0x7ffe000000000000ul
#define NATIVE  0x7fff000000000000ul
#define OBJECT  0xfffc000000000000ul

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

val_t value_to_value(val_t v);
val_t real_to_value(real_t r);
val_t small_to_value(small_t s);
val_t bool_to_value(bool_t b);
val_t glyph_to_value(glyph_t g);
val_t native_to_value(native_t n);
val_t object_to_value(void *o);

#define value(x)                                \
  _Generic((x),                                 \
           val_t:value_to_value,                \
           real_t:real_to_value,                \
           small_t:small_to_value,              \
           bool_t:bool_to_value,                \
           glyph_t:glyph_to_value,              \
           native_t:native_to_value,            \
           default:object_to_value)(x)

#endif
