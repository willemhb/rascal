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
/* forward declarations */
extern struct nul_obj_t NulObj;

/* API */
/* safe access */
 obj_head_t obj_head(obj_t obj) {
   return obj ? (obj_head_t)(obj-sizeof(struct obj_head_t)) : &NulObj.head;
}

uchar *obj_start(obj_t obj) {
  return obj? obj - obj_type_of(obj)->head_size : (uchar*)&NulObj;
}

uchar *obj_data(obj_t obj) {
  return obj ? : (uchar*)&NulObj.body[0];
}

/* predicates */
bool obj_has_type(obj_t self, type_t type) {
  if (type->isa)
    return type->isa(type, tag_val(self, OBJECT));

  return obj_type(self) == type;
}

/* object model */
type_t obj_type_of(obj_t self) {
  return obj_type(self);
}

obj_t make_obj(type_t self, size_t n, void *ini) {
  obj_t out;

  if (self->create)
    out = self->create(self, n, ini);

  else
    out = alloc(alloc_size(self, n)) + self->head_size;

  init_obj(out, self, n, ini);
  return out;
}

void init_obj(obj_t self, type_t type, size_t n, void *ini) {
  *obj_head(self) = (struct obj_head_t) { type, alloc_size(type, n), 0, true };

  if (type->init)
    type->init(self, type, n, ini);
}

obj_t resize_obj(obj_t self, size_t n) {
  if (obj_type(self)->resize)
    return obj_type(self)->resize(self, n);

  return adjust(obj_start(self), obj_size(self), alloc_size(obj_type(self), n)) + obj_type(self)->head_size;
}

void free_obj(obj_t self) {
  if (obj_type(self)->free)
    obj_type(self)->free(self);

  if (obj_allocp(self))
    dealloc(obj_start(self), obj_size(self));
    
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
