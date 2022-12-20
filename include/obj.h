#ifndef rl_obj_h
#define rl_obj_h

#include "val.h"
#include "tpl/generic.h"
#include "tpl/decl/alist.h"

/* C types */
typedef enum obj_layout_t obj_layout_t;
typedef struct obj_head_t obj_head_t;
typedef char* string_t;

enum obj_layout_t {
  allocated_obj=1,
};

/* rascal objects are pointers to the beginning of the object's data. They are immediately
   preceded by, at minimum, an object header of the following structure type. */

struct obj_head_t {
  obj_type_t type;
  char gray, black, layout, flags;
  uchar space[];
};

struct sym_t {
  ulong hash;
  ulong idno;
  string_t name;
};

struct cons_t {
  val_t car, cdr;
};

ALIST(objs, obj_t);

/* globals */
#define NUL OBJECT

extern size_t Obsize[];

extern obj_head_t nul_head; /* dummy header for nul object. */
extern val_t nul_data[2];

/* API */
/* general object API */
obj_head_t *obj_head(obj_t obj);
uchar      *obj_data(obj_t obj);

obj_type_t  obj_type(obj_t obj);
bool        obj_is_alloc(obj_t obj);
bool        has_obj_type(obj_t obj, obj_type_t type);
obj_t       make_obj(obj_type_t type);
void        free_obj(obj_t obj);

sym_t      *make_sym(char *name);
val_t       sym(char *name);

cons_t     *make_cons(val_t car, val_t cdr);
val_t       cons(val_t car, val_t cdr);

/* convenience */

/* initialization */
void obj_init( void );

/* convenience */
gen_decl(bool, is_sym);
gen_decl(sym_t*, as_sym);
gen_decl(bool, is_cons);
gen_decl(cons_t*, as_cons);

#define as_sym(x)  gen_call(x, as_sym)
#define is_sym(x)  gen_call(x, is_sym)
#define as_cons(x) gen_call(x, as_cons)
#define is_cons(x) gen_call(x, is_cons)

#endif
