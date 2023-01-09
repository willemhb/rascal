#ifndef rl_sym_h
#define rl_sym_h

#include "obj.h"

/* C types */
typedef enum sym_fl_t sym_fl_t;
typedef struct sym_head_t *sym_head_t;

enum sym_fl_t {
  bound_sym=1,
  keyword_sym=4
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
val_t lookup(char *name);
val_t bind(val_t n, val_t v);
val_t assign(val_t name, val_t val);
bool  is_bound(val_t x);
bool  is_keyword(val_t x);

/* convenience */
#define is_sym(x)   has_type(x, &SymType)
#define as_sym(x)   ((sym_t)as_obj(x))
#define sym_head(x) ((sym_head_t)obj_start(as_obj(x)))
#define sym_val(x)  (sym_head(x)->val)
#define sym_idno(x) (sym_head(x)->idno)
#define sym_hash(x) (sym_head(x)->hash)

#endif
