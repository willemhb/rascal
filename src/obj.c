#include <assert.h>

#include "obj.h"

#include "memory.h"

#include "util/hashing.h"
#include "util/string.h"

/* commentary */

/* C types */
#include "tpl/decl/hashmap.h"
HASHMAP(symbol_table, char*, sym_t*);

bool symt_cmp(string_t stringX, string_t stringY) {
  return strcmp(stringX, stringY) == 0;
}

sym_t *symt_intern(char *string, void **space) {

  sym_t *out = make_sym(string);

  space[0]   = out->name;
  space[1]   = out;

  return out;
}

#include "tpl/impl/hashmap.h"
#include "tpl/type.h"

HASHMAP(symbol_table, char*, sym_t*, pad_table_size, hash_str8, symt_cmp, symt_intern, NULL, NULL);

/* globals */
obj_head_t nul_head = { nul_obj, true, false, 0, 0 };
val_t nul_data[2] = { NUL, NUL };

size_t Obsize[] = {
  [nul_obj]  = 0,
  [sym_obj]  = sizeof(obj_head_t) + sizeof(sym_t),
  [cons_obj] = sizeof(obj_head_t) + sizeof(cons_t)
};

symbol_table_t SymbolTable;

/* API */
/* describe */
#define gen_isa(type)					\
  bool val_is_##type(val_t val) {			\
    return has_rl_type(val, type##_type);		\
  }							\
  							\
  bool obj_is_##type(obj_t obj) {			\
    return has_obj_type(obj, type##_obj);		\
  }

#define gen_asa(type, cnvt)				\
  TYPE(type) *val_as_##type(val_t val) {		\
    assert(val_is_##type(val));			\
    return (TYPE(type)*)cnvt(val);		\
  }						\
  						\
  TYPE(type) *obj_as_##type(obj_t obj) {	\
    assert(obj_is_##type(obj));			\
    return (TYPE(type)*)obj;			\
  }
  
/* general object API */
 obj_head_t *obj_head(obj_t obj)
{
  if (obj)
    return (obj_head_t*)(obj-sizeof(obj_head_t));

  return &nul_head;
}

uchar *obj_data(obj_t obj)
{
  return obj ? : (uchar*)&nul_data[0];
}

obj_type_t obj_type(obj_t obj) {
  return obj_head(obj)->type;
}

bool obj_is_alloc(obj_t obj) {
  return flagp(obj_head(obj)->flags, allocated_obj);
}

bool has_obj_type(obj_t obj, obj_type_t type) {
  return obj_type(obj) == type;
}

obj_t make_obj(obj_type_t type) {
  assert(type != nul_obj);

  size_t total = Obsize[type];
  obj_head_t *head = alloc(total);

  *head = (obj_head_t) { type, true, false, allocated_obj, 0 };

  return head->space;
}

void free_obj(obj_t obj) {
  obj_head_t *to_free = obj_head(obj);
  size_t size = Obsize[to_free->type];

  if (obj_is_alloc(obj))
    dealloc(to_free, size);
}

/* symbol API */
sym_t *make_sym(char *name) {
  static ulong counter = 1;

  sym_t *new_sym = (sym_t*)make_obj(sym_obj);

  new_sym->idno = counter++;
  new_sym->hash = hash_str8(name);
  new_sym->name = make_string(strlen(name), name);

  return new_sym;
}

val_t  sym(char *name) {
  sym_t *out = symbol_table_intern(&SymbolTable, name);

  return tag_val(out, OBJECT);
}

gen_isa(sym);
gen_asa(sym, as_obj);

/* cons API */
cons_t *make_cons(val_t car, val_t cdr) {
  cons_t *out = (cons_t*)make_obj(cons_obj);
  out->car    = car;
  out->cdr    = cdr;

  return out;
}

val_t cons(val_t car, val_t cdr) {
  cons_t *out = make_cons(car, cdr);

  return tag_val(out, OBJECT);
}

/* initialization */
void obj_init( void ) {
  init_symbol_table(&SymbolTable, 0, NULL);
}
