#ifndef rl_arr_h
#define rl_arr_h

#include <assert.h>
/* generic array object API methods */

#include "obj.h"

/* C types */
typedef struct arr_head_t *arr_head_t;

struct arr_head_t {
  size_t len, cap;
  struct obj_head_t obj;
};

/* globals */
extern struct type_t ArrType;

/* API */
void   init_arr(obj_t self, type_t type, size_t n, void *ini);
obj_t  create_arr(type_t type, size_t n, void *ini);
obj_t  resize_arr(obj_t self, size_t n);

/* convenience */
#define arr_head(arr) ((arr_head_t)obj_start((obj_t)(arr)))
#define is_arr(x) has_type(x, &ArrType)

#define assert_bound(i, a) assert((i) >= 0 && (size_t)(i) < arr_head(a)->len)

#endif
