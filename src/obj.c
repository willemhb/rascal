#include <assert.h>

#include "obj.h"
#include "type.h"
#include "memory.h"

#include "util/hashing.h"
#include "util/string.h"

/* commentary */

/* C types */
#include "tpl/impl/alist.h"
ALIST(objs, obj_t, obj_t, pad_alist_size);

/* globals */
obj_head_t NulHead = { nul_obj, true, false, 0, 0 };
val_t NulBody[2] = {  OBJECT, OBJECT };

/* API */
/* safe access */
 obj_head_t *obj_head(obj_t obj) {
   return obj ? (obj_head_t*)(obj-sizeof(obj_head_t)) : &NulHead;
}

uchar *obj_start(obj_t obj) {
  return obj? obj - head_size_for(obj) : (uchar*)&NulHead;
}

uchar *obj_data(obj_t obj) {
  return obj ? : (uchar*)&NulBody[0];
}

/* predicates */
bool obj_is_alloc(obj_t obj) {
  return flagp(obj_head(obj)->flags, allocated_obj);
}

bool has_obj_type(obj_t obj, obj_type_t type) {
  return obj_type(obj) == type;
}

/* object model API */
size_t obj_size(obj_t self) {
  if (objsize_for(self))
    return objsize_for(self)(self);

  return base_size_for(self);
}

obj_t make_obj(obj_type_t type, size_t n, void *ini) {
  obj_t new;

  if (create_for(type))
    new = create_for(type)(type, n, ini);

  else
    new = alloc(base_size_for(type)) + base_offset_for(type);

  init_obj(new, type, n, ini);

  return new;
}

void init_obj(obj_t self, obj_type_t type, size_t n, void *ini) {
  *obj_head(self) = (obj_head_t) { type, true, false, allocated_obj, 0 };

  if (init_for(type))
    init_for(type)(self, type, n, ini);
}

void free_obj(obj_t self) {
  if (free_for(self))
    free_for(self)(self);

  if (obj_is_alloc(self))
    dealloc(obj_start(self), obj_size(self));
}

obj_t resize_obj(obj_t self, size_t n) {
  if (resize_for(self))
    return resize_for(self)(self, n);

  return adjust(self, obj_size(self), base_size_for(self) + n);
}

/* initialization */
extern void sym_init(void);
extern void list_init(void);
extern void vec_init(void);
extern void num_init(void);
extern void code_init(void);
extern void func_init(void);

void obj_init(void) {
  sym_init();
  list_init();
  vec_init();
  num_init();
  code_init();
  func_init();
}
