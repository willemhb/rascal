#include "set.h"

#include "prin.h"
#include "type.h"

/* globals */
void  prin_set(val_t s);
int   compare_sets(val_t sx, val_t sy, void *state);
bool  equal_sets(val_t sx, val_t sy, void *state);
obj_t create_set(type_t type, size_t n, void *ini);
void  init_set(obj_t self, type_t type, size_t n, void *ini);
void  free_set(obj_t self);
obj_t resize_set(obj_t self, size_t new_size);

struct type_t SetType = {
  .name="set",
  .idno=set_type_idno,

  .prin=prin_set,
  .compare=compare_sets,
  .equal=equal_sets,

  .create=create_set,
  .resize=resize_set,
  .init=init_set,
  .free=free_set
};

/* API */
set_t make_set(void) {
  return (set_t)make_obj(&SetType, 0, NULL);
}

void prin_set(val_t s) {
  set_t sx = as_set(s);
  printf("{");

  for (size_t i=0; i<sx->nkeys; i++) {
    val_t v = vals_ref(sx->entries, i); prin(v);
    if (i+1 < sx->nkeys)
      printf(" ");
  }

  printf("}");
}
