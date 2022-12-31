#ifndef rl_sym_h
#define rl_sym_h

#include "obj.h"

/* C types */
typedef enum sym_fl_t sym_fl_t;
typedef struct sym_head_t *sym_head_t;

enum sym_fl_t {
  bound_sym=1
};

struct sym_head_t {
  val_t val;
  ulong hash, idno;
  struct obj_head_t obj;
};

/* globals */
extern struct type_t SymType;

/* API */
sym_t make_sym(char *name);
val_t sym(char *name);
val_t define(char *name, val_t val);
bool  is_bound(val_t x);
val_t assign(val_t name, val_t val);

/* convenience */
#define is_sym(x)   has_type(x, &SymType)
#define as_sym(x)   ((sym_t)as_obj(x))
#define sym_head(x) ((sym_head_t)obj_start(as_obj(x)))

#endif
