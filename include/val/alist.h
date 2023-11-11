#ifndef rl_val_alist_h
#define rl_val_alist_h

#include "val/object.h"

/* */

/* C types */
struct Alist {
  HEADER;
  Value* data;
  size_t cnt;
  size_t cap;
};

struct Objects {
  HEADER;
  Obj*   data;
  size_t cnt;
  size_t cap;
};

/* globals */
extern struct Type AlistType, ObjectsType;

/* external API */
Alist  new_alist(Dict meta);
void   init_alist(Alist arr);
void   free_alist(Alist arr);
void   resize_alist(Alist arr, size_t new_size);
size_t alist_push(Alist arr, Value x);
Value  alist_pop(Alist arr);

Objects new_objects(Dict meta);
void    init_objects(Objects slf);



#endif
