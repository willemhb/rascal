#ifndef rl_val_alist_h
#define rl_val_alist_h

#include "val/object.h"

/* Mutable dynamic array types. */

/* C types */
struct Alist {
  HEADER;
  Value* data;
  size_t cnt;
  size_t cap;
};

struct Objects {
  HEADER;
  Obj**  data;
  size_t cnt;
  size_t cap;
};

/* Globals */
extern struct Type AlistType, ObjectsType;

/* External APIs */
Alist*   new_alist(void);
void     init_alist(Alist* arr);
void     free_alist(Alist* arr);
void     resize_alist(Alist* arr, size_t new_cnt);
size_t   alist_push(Alist* arr, Value x);
size_t   alist_write(Alist* arr, size_t n, Value* data);
size_t   alist_pushn(Alist* arr, size_t n, ...);
Value    alist_pop(Alist* arr);
Value    alist_popn(Alist* arr, size_t n);

Objects* new_objects(void);
void     init_objects(Objects* arr);
void     free_objects(Objects* arr);
void     resize_objects(Objects* arr, size_t new_cnt);
size_t   objects_push(Objects* arr, Obj* o);
size_t   objects_write(Objects* arr, size_t n, Obj** data);
size_t   objects_pushn(Objects* arr, size_t n, ...);
Obj*     objects_pop(Objects* arr);
Obj*     objects_popn(Objects* arr, size_t n);

#endif
