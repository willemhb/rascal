#include <stdlib.h>

#include "func.h"

#include "type.h"
#include "memory.h"

#include "prin.h"

/* globals */
bool isa_func(type_t self, val_t val);

struct type_t FuncType = {
  .name="func",
  .isa =isa_func
};

/* API */
/* internal */
void init_func(obj_t self, type_t type, size_t n, void *ini) {
  (void)type;
  (void)n;

  if (ini) {
    func_head_t head = func_head(self);
    func_init_t init = ini;

    head->name  = init->name;
    head->type  = init->type;
    head->nargs = init->nargs;
    head->vargs = init->vargs;
    head->guard = init->guard;
  }
}

void prin_func(val_t val) {
  obj_t obj   = as_obj(val);
  char *name  = func_head(obj)->name;
  char *tname = obj_type(obj)->name;
  int   arity = func_head(obj)->nargs;

  printf("#%s(%s %d)", tname, name, arity);
}

extern struct type_t NativeType, PrimType, ModuleType;

bool isa_func(type_t self, val_t val) {
  self = type_of(val);

  return self == &NativeType || self == &PrimType || self == &ModuleType;
}

/* initialization */
extern void prim_init(void);
extern void native_init(void);
extern void module_init(void);

void func_init(void) {
  prim_init();
  native_init();
  module_init();
}
