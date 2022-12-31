#include <assert.h>
#include <string.h>

#include "sym.h"

#include "val.h"
#include "type.h"
#include "memory.h"

#include "prin.h"

#include "util/collection.h"

/* C types */
/* symbol table implementation */
#include "tpl/decl/hashmap.h"
HASHMAP(symbol_table, char*, sym_t);

bool symt_cmp(char* stringX, char* stringY) {
  return strcmp(stringX, stringY) == 0;
}

sym_t symt_intern(char *string, void **space) {

  sym_t out = make_sym(string);

  space[0]   = out;
  space[1]   = out;

  return out;
}

#include "tpl/impl/hashmap.h"
HASHMAP(symbol_table, char*, sym_t, pad_table_size, hash_str8, symt_cmp, symt_intern, NULL, NULL);

/* globals */
/* API methods */
void prin_sym(val_t val);
void init_sym(obj_t self, type_t type, size_t n, void* ini);

symbol_table_t SymbolTable;

struct type_t SymType = {
  .name="sym",

  .prin=prin_sym,

  .init=init_sym,
  .pad=pad_string_size,

  .head_size=sizeof(struct sym_head_t),
  .base_offset=sizeof(struct sym_head_t) - sizeof(struct obj_head_t),
  .el_size=1
};

/* API */
/* external */
sym_t make_sym(char *name) {
  size_t name_len = strlen(name);

  return (sym_t)make_obj(&SymType, name_len, name);
}

val_t sym(char *name) {
  sym_t out = symbol_table_intern(&SymbolTable, name);

  return tag_val(out, OBJECT);
}

val_t define(char *name, val_t val) {
  val_t symname               = sym(name);
  sym_head(symname)->val      = val;
  obj_flags(as_obj(symname)) |= bound_sym;

#ifdef DEBUG
  // printf("name: %s, val: ", as_sym(symname));
  // prin(val);
  // printf("\n");
#endif

  return val;
}

val_t assign(val_t name, val_t val) {
  assert(is_sym(name));
  assert(is_bound(name));

  sym_head(name)->val = val;

  return val;
}

bool is_bound(val_t x) {
  return is_sym(x) && flagp(obj_flags(as_obj(x)), bound_sym);
}

/* internal */
void init_sym(obj_t self, type_t type, size_t n, void *ini) {
  static ulong counter = 1;
  
  (void)n;
  (void)type;

  sym_head(self)->val  = OBJECT;
  sym_head(self)->idno = counter++;
  sym_head(self)->hash = hash_str8(ini);

  strcpy((char*)self, ini);
}

void prin_sym(val_t x) {
  printf("%s", as_sym(x));
}

/* native functions */
#include "native.h"
#include "bool.h"

#include "tpl/impl/funcall.h"

func_err_t guard_lookup(size_t nargs, val_t *args) {
  (void)nargs;

  TYPE_GUARD(sym, args, 0);
  TYPE_GUARD(bound, args, 0);

  return func_no_err;
}

func_err_t sym_method_guard(size_t nargs, val_t *args) {
  (void)nargs;

  TYPE_GUARD(sym, args, 0);

  return func_no_err;
}

val_t native_lookup(size_t nargs, val_t *args) {
  (void)nargs;

  return sym_head(as_sym(args[0]))->val;
}

val_t native_boundp(size_t nargs, val_t *args) {
  (void)nargs;

  if (is_bound(args[0]))
    return TRUE;

  return FALSE;
}

val_t native_sym(size_t nargs, val_t *args) {
  (void)nargs;

  return args[0];
}


/* initialization */
void sym_init( void ) {
  // create symbol table
  init_symbol_table(&SymbolTable, 0, NULL);

  // native functions
  def_native("sym", 1, false, sym_method_guard, &SymType, native_sym);
  def_native("bound?", 1, false, sym_method_guard, NULL, native_boundp);
  def_native("lookup", 1, false, guard_lookup, NULL, native_lookup);
}
