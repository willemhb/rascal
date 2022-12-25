#include <assert.h>
#include <stdarg.h>

#include "vec.h"

#include "arr.h"

#include "memory.h"

#include "prin.h"

#include "util/collection.h"

/* C types */

/* globals */

/* API */
/* external */
vec_t make_vec(size_t n, val_t *vals) {
  return (vec_t)make_obj(vec_obj, n, vals);
}

val_t vec(size_t n, ...) {
  val_t buf[n];

  va_list va; va_start(va, n);

  for (size_t i=0; i<n; i++)
    buf[i] = va_arg(va, val_t);

  va_end(va);

  vec_t v = make_vec(n, buf);

  return tag_val(v, OBJECT);
}

val_t vec_ref(vec_t vec, int i) {
  if (i < 0)
    i += vec_len(vec);

  assert(i >= 0 && (size_t)i < vec_len(vec));
  return vec[i];
}

val_t vec_set(vec_t vec, int i, val_t val) {
  if (i < 0)
    i += vec_len(vec);

  assert(i >= 0 && (size_t)i < vec_len(vec));
  vec[i] = val;
  return val;
}

size_t vec_push(vec_t *vec, val_t x) {
  size_t out = vec_len(*vec);
  *vec = (vec_t)resize_obj(*(obj_t*)vec, out+1);
  vec_set(*vec, -1, x);
  return out;
}

val_t vec_pop(vec_t *vec) {
  assert(vec_len(*vec) > 0);
  val_t out = vec_ref(*vec, -1);
  *vec = (vec_t)resize_obj(*(obj_t*)vec, vec_len(*vec)-1);

  return out;
}

/* internal */
void prin_vec(val_t x) {
  printf("[");

  vec_t  v = as_vec(x);
  size_t l = vec_len(v);

  for (size_t i=0; i < l; i++) {
    prin(v[i]);
    
    if (i+1 < l)
      printf(" ");
  }

  printf("]");
}

/* generics */
#include "tpl/impl/generic.h"

ISA_METHOD(vec, val, rl_type, 1, vec_type);
ISA_METHOD(vec, obj, rl_type, 1, vec_type);
ISA_NON0(vec, vec);

ASA_METHOD(vec, val, is_vec, as_obj);
ASA_METHOD(vec, obj, is_vec, NOOP_CNVT);
ASA_METHOD(vec, vec, NON0_GUARD, NOOP_CNVT);

HEAD_METHOD(vec, val, is_vec, as_obj);
HEAD_METHOD(vec, obj, is_vec, NOOP_CNVT);
HEAD_METHOD(vec, vec, NON0_GUARD, NOOP_CNVT);

/* initialization */
void vec_init(void) {
  Type[vec_type] = (struct dtype_t) {
    .name="vec",
    .prin=prin_vec,

    .create=create_arr,
    .init=init_arr,
    .objsize=arr_size,
    .resize=resize_arr,

    .head_size=sizeof(struct vec_head_t),
    .body_size=0,
    .base_offset=sizeof(struct vec_head_t) - sizeof(struct obj_head_t),
    .el_size=sizeof(val_t)
  };
}
