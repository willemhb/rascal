#ifndef rl_sym_h
#define rl_sym_h

#include "obj.h"

/* C types */
typedef enum sym_fl_t sym_fl_t;

enum sym_fl_t {
  bound_sym=128
};

struct sym_head_t {
  val_t val;
  ulong hash, idno;
  obj_head_t obj;
};

/* globals */

/* API */
sym_t make_sym(char *name);
val_t sym(char *name);
val_t define(char *name, val_t val);

/* generics */
#include "tpl/decl/generic.h"

#define is_sym(x)   GENERIC_CALL_3(is_sym, val, obj, sym, x)
#define as_sym(x)   GENERIC_CALL_3(as_sym, val, obj, sym, x)
#define is_bound(x) GENERIC_CALL_3(is_bound, val, obj, sym, x)
#define sym_head(x) GENERIC_CALL_3(sym_head, val, obj, sym, x)

#define sym_hash(x) GETF_3(hash, sym_head, val, obj, sym, x)
#define sym_idno(x) GETF_3(idno, sym_head, val, obj, sym, x)
#define sym_val(x)  GETF_3(val, sym_head, val, obj, sym, x)

bool METHOD(val, is_sym)(val_t val);
bool METHOD(obj, is_sym)(obj_t obj);
bool METHOD(sym, is_sym)(sym_t sym);

sym_t METHOD(val, as_sym)(val_t val);
sym_t METHOD(obj, as_sym)(obj_t obj);
sym_t METHOD(sym, as_sym)(sym_t sym);

bool METHOD(val, is_bound)(val_t val);
bool METHOD(obj, is_bound)(obj_t obj);
bool METHOD(sym, is_bound)(sym_t sym);

struct sym_head_t *METHOD(val, sym_head)(val_t val);
struct sym_head_t *METHOD(obj, sym_head)(obj_t obj);
struct sym_head_t *METHOD(sym, sym_head)(sym_t sym);

/* initialization */
void  sym_init(void);


#endif
