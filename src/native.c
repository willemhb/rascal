#include "native.h"

#include "type.h"

/* C types */
typedef struct native_init_t *native_init_t;

struct native_init_t {
  struct func_init_t func_init;
  native_fn_t funptr;
};

/* globals */
void init_native(obj_t self, type_t type, size_t n, void *ini);

struct type_t NativeType = {
  .name="native",
  .prin=prin_func,
  .init=init_native,
  .head_size=sizeof(struct func_head_t),
  .body_size=sizeof(struct native_t),
  .base_offset=sizeof(struct func_head_t) - sizeof(struct obj_head_t)
};

/* API */
/* external */
native_t make_native(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, native_fn_t funptr) {
  struct native_init_t init = {
    .func_init = {
      .name=name,
      .nargs=nargs,
      .vargs=vargs,
      .guard=guard,
      .type=type
    },
    .funptr=funptr
  };

  return (native_t)make_obj(&NativeType, 6, &init);
}

val_t native(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, native_fn_t funptr) {
  native_t out = make_native(name, nargs, vargs, guard, type, funptr);

  return tag_val(out, OBJECT);
}

/* internal */
void init_native(obj_t self, type_t type, size_t n, void *ini) {
  init_func(self, type, n, ini);

  if (ini)
    as_native(self)->funptr = ((native_init_t)ini)->funptr;
}

/* initialization */
void native_init(void) {}
