#ifndef rl_obj_h
#define rl_obj_h

#include "val.h"

/* C types */
typedef enum obj_flag_t obj_flag_t;
typedef struct obj_head_t obj_head_t;

enum obj_flag_t {
  allocated_obj=1,
};

struct obj_head_t {
  obj_type_t type;
  char gray, black, layout, flags;
  uchar space[];
};

/* object alist for internal use */
#include "tpl/decl/alist.h"

ALIST(objs, obj_t);

/* globals */
/* API */
/* safe access */
obj_head_t *obj_head(obj_t obj);
uchar      *obj_data(obj_t obj);
uchar      *obj_start(obj_t obj);

/* predicates */
bool obj_is_alloc(obj_t obj);
bool has_obj_type(obj_t obj, obj_type_t type);

/* object model API */
size_t obj_size(obj_t self);
obj_t  make_obj(obj_type_t type, size_t n, void *ini);
void   init_obj(obj_t self, obj_type_t type, size_t n, void *ini);
void   free_obj(obj_t self);
obj_t  resize_obj(obj_t self, size_t n);

/* initialization */
void obj_init(void);

/* convenience */
#define obj_type(x)   (obj_head(x)->type)
#define obj_flags(x)  (obj_head(x)->flags)
#define obj_layout(x) (obj_head(x)->layout)
#define obj_gray(x)   (obj_head(x)->gray)
#define obj_black(x)  (obj_head(x)->black)

#endif
