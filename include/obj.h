#ifndef rl_obj_h
#define rl_obj_h

#include "val.h"

/* C types */
typedef struct obj_head_t *obj_head_t;

struct obj_head_t {
  type_t type;
  size_t size   : 48;
  size_t flags  : 15;
  size_t allocp :  1;
  uchar space[];
};

struct nul_obj_t {
  struct obj_head_t head;
  val_t body[2];
};

/* object alist for internal use */
#include "tpl/decl/alist.h"

ALIST(objs, obj_t);

/* globals */
/* API */
/* safe access */
obj_head_t  obj_head(obj_t obj);
uchar      *obj_data(obj_t obj);
uchar      *obj_start(obj_t obj);

/* predicates */
bool obj_has_type(obj_t obj, type_t type);

/* object model API */
type_t obj_type_of(obj_t self);
obj_t  make_obj(type_t type, size_t n, void *ini);
void   init_obj(obj_t self, type_t type, size_t n, void *ini);
obj_t  resize_obj(obj_t self, size_t n);
void   free_obj(obj_t self);

/* initialization */
void obj_init(void);

/* convenience */
#define obj_type(x)   (obj_head(x)->type)
#define obj_size(x)   (obj_head(x)->size)
#define obj_flags(x)  (obj_head(x)->flags)
#define obj_allocp(x) (obj_head(x)->allocp)

#endif
