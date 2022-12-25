#ifndef rl_type_h
#define rl_type_h

#include "rascal.h"

/* C types */
struct dtype_t {
  /* general metadata */
  char        *name;

  /* common methods */
  prin_fn_t    prin;

  /* object methods */
  create_fn_t  create;
  resize_fn_t  resize;
  init_fn_t    init;
  objsize_fn_t objsize;
  runtime_fn_t free;
  pad_fn_t     pad;

  /* object layout */
  size_t head_size;
  size_t base_offset;
  size_t body_size;
  size_t el_size;
};

/* globals */
extern struct dtype_t Type[num_types];

/* API */
#include "tpl/decl/generic.h"

#define dtype_for(x)            GENERIC_CALL_5(dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define dtype_of(x)             GENERIC_CALL_2(dtype_of, obj, val, x)
#define base_size_for(x)        GENERIC_CALL_5(base_size_for, dtype, rl_type, obj_type, obj, val, x)
#define rl_type(x)              GENERIC_CALL_2(rl_type, val, obj, x)
#define has_rl_type(x, t)       GENERIC_CALL_2(has_rl_type, val, obj, x, t)
#define has_rl_types(x, n, ...) GENERIC_CALL_2(has_rl_types, val, obj, (x), (n) __VA_OPT__(,) __VA_ARGS__)

rl_type_t METHOD(val, rl_type)(val_t val);
rl_type_t METHOD(obj, rl_type)(obj_t obj);

bool METHOD(val, has_rl_type)(val_t val, rl_type_t type);
bool METHOD(obj, has_rl_type)(obj_t obj, rl_type_t type);

bool METHOD(val, has_rl_types)(val_t val, size_t n, ...);
bool METHOD(obj, has_rl_types)(obj_t obj, size_t n, ...);

dtype_t METHOD(dtype, dtype_for)(dtype_t dtype);
dtype_t METHOD(obj_type, dtype_for)(obj_type_t obj_type);
dtype_t METHOD(rl_type, dtype_for)(rl_type_t rl_type);
dtype_t METHOD(obj, dtype_for)(obj_t obj);
dtype_t METHOD(val, dtype_for)(val_t val);

dtype_t METHOD(obj, dtype_of)(obj_t obj);
dtype_t METHOD(val, dtype_of)(val_t val);

size_t METHOD(dtype, base_size_for)(dtype_t dtype);
size_t METHOD(obj_type, base_size_for)(obj_type_t obj_type);
size_t METHOD(rl_type, base_size_for)(rl_type_t rl_type);
size_t METHOD(obj, base_size_for)(obj_t obj);
size_t METHOD(val, base_size_for)(val_t val);

#define name_for(x)        GETF_5(name, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define prin_for(x)        GETF_5(prin, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define create_for(x)      GETF_5(create, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define resize_for(x)      GETF_5(resize, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define init_for(x)        GETF_5(init, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define free_for(x)        GETF_5(free, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define objsize_for(x)     GETF_5(objsize, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define pad_for(x)         GETF_5(pad, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define head_size_for(x)   GETF_5(head_size, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define base_offset_for(x) GETF_5(base_offset, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define body_size_for(x)   GETF_5(body_size, dtype_for, dtype, rl_type, obj_type, obj, val, x)
#define el_size_for(x)     GETF_5(el_size, dtype_for, dtype, rl_type, obj_type, obj, val, x)

#endif
