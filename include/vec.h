#ifndef rl_vec_h
#define rl_vec_h

#include "obj.h"

/* C types */
struct vec_head_t {
  size_t len, cap;
  obj_head_t obj;
};

/* API */
vec_t       make_vec(size_t n, val_t *vals);
val_t       vec(size_t n, ...);
val_t       vec_ref(vec_t vec, int n);
val_t       vec_set(vec_t vec, int n, val_t x);
size_t      vec_push(vec_t *vec, val_t x);
val_t       vec_pop(vec_t *vec);

/* generics */
#include "tpl/decl/generic.h"
bool METHOD(val, is_vec)(val_t val);
bool METHOD(obj, is_vec)(obj_t obj);
bool METHOD(vec, is_vec)(vec_t vec);

vec_t METHOD(val, as_vec)(val_t val);
vec_t METHOD(obj, as_vec)(obj_t obj);
vec_t METHOD(vec, as_vec)(vec_t vec);

struct vec_head_t *METHOD(val, vec_head)(val_t val);
struct vec_head_t *METHOD(obj, vec_head)(obj_t obj);
struct vec_head_t *METHOD(vec, vec_head)(vec_t vec);

#define is_vec(x)   GENERIC_CALL_3(is_vec, val, obj, vec, x)
#define as_vec(x)   GENERIC_CALL_3(as_vec, val, obj, vec, x)
#define vec_head(x) GENERIC_CALL_3(vec_head, val, obj, vec, x)
#define vec_len(x)  GETF_3(len, vec_head, val, obj, vec, x)
#define vec_cap(x)  GETF_3(cap, vec_head, val, obj, vec, x)

/* initialization */
void vec_init(void);

#endif
