#ifndef rl_arr_h
#define rl_arr_h

/* generic array object API methods */

#include "obj.h"

/* C types */
struct arr_head_t {
  size_t len, cap;
  obj_t obj;
};

/* API */
size_t arr_size(obj_t self);
void   init_arr(obj_t self, obj_type_t type, size_t n, void *ini);
obj_t  create_arr(obj_type_t type, size_t n, void *ini);
obj_t  resize_arr(obj_t self, size_t n);

/* convenience */
#define arr_head(arr) ((struct arr_head_t*)((arr)-sizeof(struct arr_head_t)))
#define arr_len(arr)  (arr_head(arr)->len)
#define arr_cap(arr)  (arr_head(arr)->cap)

#endif
