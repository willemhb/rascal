#include <assert.h>
#include <stdarg.h>

#include "vec.h"

#include "arr.h"

#include "type.h"
#include "memory.h"

#include "prin.h"

#include "util/collection.h"

/* C types */

/* globals */
void prin_vec(val_t val);

struct type_t VecType = {
  .name="vec",
  .prin=prin_vec,
  .create=create_arr,
  .init=init_arr,
  .resize=resize_arr,
  .pad=pad_alist_size,
  .head_size=sizeof(struct vec_head_t),
  .base_offset=sizeof(struct vec_head_t) - sizeof(struct obj_head_t),
  .el_size=sizeof(val_t)
};

/* API */
/* external */
vec_t make_vec(size_t n, val_t *vals) {
  return (vec_t)make_obj(&VecType, n, vals);
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
  vec_head_t head = vec_head(vec);
  
  if (i < 0)
    i += head->len;

  assert_bound(i, head->len);
  return vec[i];
}

val_t vec_set(vec_t vec, int i, val_t val) {
  vec_head_t head = vec_head(vec);
  
  if (i < 0)
    i += head->len;

  assert_bound(i, head->len);
  vec[i] = val;
  return val;
}

size_t vec_push(vec_t *vec, val_t x) {
  size_t out = vec_head(*vec)->len;
  *vec = (vec_t)resize_obj(*(obj_t*)vec, out+1);
  vec_set(*vec, -1, x);
  return out;
}

val_t vec_pop(vec_t *vec) {
  assert(vec_head(*vec)->len > 0);
  val_t out = vec_ref(*vec, -1);
  *vec = (vec_t)resize_obj(*(obj_t*)vec, vec_head(*vec)->len-1);

  return out;
}

/* internal */
void prin_vec(val_t x) {
  printf("[");

  vec_t  v = as_vec(x);
  size_t l = vec_head(v)->len;

  for (size_t i=0; i < l; i++) {
    prin(v[i]);
    
    if (i+1 < l)
      printf(" ");
  }

  printf("]");
}

/* native functions */
/* native functions */
#include "sym.h"
#include "native.h"

#include "tpl/impl/funcall.h"

func_err_t vec_accessor_guard(size_t nargs, val_t *args) {
  (void)nargs;

  TYPE_GUARD(vec, args, 0);
  INDEX_GUARD(vec_head(args[0])->len, args, 1);

  return func_no_err;
}

func_err_t vec_method_guard(size_t nargs, val_t *args) {
  (void)nargs;

  TYPE_GUARD(vec, args, 0);

  return func_no_err;
}

func_err_t vec_pop_guard(size_t nargs, val_t *args) {
  (void)nargs;
  
  TYPE_GUARD(vec, args, 0);

  if (vec_head(args[0])->len == 0)
    return func_arg_value_err;

  return func_no_err;
}

val_t native_vec(size_t nargs, val_t *args) {
  return vec(nargs, args);
}

val_t native_vec_ref(size_t nargs, val_t *args) {
  (void)nargs;
  
  return vec_ref(as_vec(args[0]), as_small(args[1]));
}

val_t native_vec_set(size_t nargs, val_t *args) {
  (void)nargs;

  return vec_set(as_vec(args[0]), as_small(args[1]), args[2]);
}

val_t native_vec_len(size_t nargs, val_t *args) {
  (void)nargs;

  small_t out = vec_head(args[0])->len;

  return tag_val(out, SMALL);
}

void vec_init(void) {
  def_native("vec", 0, true, NULL, &VecType, native_vec);
  def_native("vec-ref", 2, false, vec_accessor_guard, NULL, native_vec_ref);
  def_native("vec-xef", 3, false, vec_accessor_guard, NULL, native_vec_set);
  def_native("vec-len", 1, false, vec_method_guard, NULL, native_vec_len);
}
