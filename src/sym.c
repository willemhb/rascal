#include <string.h>

#include "sym.h"

#include "val.h"
#include "type.h"
#include "memory.h"

#include "prin.h"

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
symbol_table_t SymbolTable;

/* API */
/* external */
sym_t make_sym(char *name) {
  size_t name_len = strlen(name);

  return (sym_t)make_obj(sym_obj, name_len, name);
}

val_t sym(char *name) {
  sym_t out = symbol_table_intern(&SymbolTable, name);

  return tag_val(out, OBJECT);
}

val_t define(char *name, val_t val) {
  val_t symname = sym(name);
  sym_val(symname) = val;

  return val;
}

/* internal */
obj_t create_sym(obj_type_t type, size_t n, void *ini) {
  (void)ini;

  return alloc(head_size_for(type)+n+1) + base_offset_for(type);
}

void init_sym(obj_t self, obj_type_t type, size_t n, void *ini) {
  static ulong counter = 1;

  (void)type;
  (void)n;

  struct sym_head_t *head = sym_head((sym_t)self);

  head->idno = counter++;
  head->hash = hash_str8(ini);

  strcpy((char*)self, ini);
}

size_t sym_size(obj_t self) {
  return sizeof(struct sym_head_t) + strlen((char*)self) + 1;
}
  
void prin_sym(val_t x) {
  printf("%s", as_sym(x));
}

/* generics methods */
#include "tpl/impl/generic.h"

ISA_METHOD(sym, val, rl_type, 1, sym_type);
ISA_METHOD(sym, obj, r_type, 1, sym_type);
ISA_NON0(sym, sym);

ASA_METHOD(sym, val, is_obj, as_obj);
ASA_METHOD(sym, obj, is_sym, as_sym);
ASA_METHOD(sym, sym, NON0_GUARD, NOOP_CNVT);

FLAGP_METHOD(sym, val, OBJ_GETFL, bound);
FLAGP_METHOD(sym, obj, OBJ_GETFL, bound);
FLAGP_METHOD(sym, sym, OBJ_GETFL, bound);

HEAD_METHOD(sym, val, is_sym, as_sym);
HEAD_METHOD(sym, obj, is_sym, as_sym);
HEAD_METHOD(sym, sym, is_sym, as_sym);

/* initialization */
void sym_init( void ) {
  init_symbol_table(&SymbolTable, 0, NULL);

  Type[sym_type] = (struct dtype_t) {
    .name="sym",
    .prin=prin_sym,

    /* object methods */
    .create=create_sym,
    .init=init_sym,
    .objsize=sym_size,

    /* layout */
    .head_size=sizeof(struct sym_head_t),
    .base_offset=sizeof(struct sym_head_t) - sizeof(struct obj_head_t),
    .body_size=0
  };
}
