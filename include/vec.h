#ifndef rl_vec_h
#define rl_vec_h

#include "arr.h"

/* C types */
typedef struct vec_head_t *vec_head_t;

struct vec_head_t {
  size_t len, cap;
  struct obj_head_t obj;
};

/* globals */
extern struct type_t VecType;

/* API */
vec_t       make_vec(size_t n, val_t *vals);
val_t       vec(size_t n, ...);
val_t       vec_ref(vec_t vec, int n);
val_t       vec_set(vec_t vec, int n, val_t x);
size_t      vec_push(vec_t *vec, val_t x);
val_t       vec_pop(vec_t *vec);

/* initialization */
void vec_init(void);

/* convenience */
#define is_vec(x)   has_type(x, &VecType)
#define as_vec(x)   ((vec_t)as_obj(x))
#define vec_head(x) ((vec_head_t)obj_start((obj_t)(x)))

#endif
