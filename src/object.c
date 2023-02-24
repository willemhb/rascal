#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "object.h"
#include "compare.h"
#include "runtime.h"

#include "util/hash.h"
#include "util/collections.h"

/* globals */
uidno SymbolCounter = 1;
Sym*  SymbolTable = NULL;

List   EmptyList = {
  .next_live=NULL,
  .type=LIST,
  .flags=FROZEN|GRAY,
  .hash=0,
  .len=0,
  .head=NUL,
  .tail=&EmptyList
};

Str EmptyString = {
  .next_live=NULL,
  .type=STR,
  .flags=FROZEN|ENCODED,
  .len=0,
  .hash=0,
  .chars={ '\0' }
};

Tuple EmptyTuple = {
  .next_live=NULL,
  .type=TUPLE,
  .flags=FROZEN,
  .len=0
};

// API ------------------------------------------------------------------------
// common utilities -----------------------------------------------------------
// lifetime & memory management -----------------------------------------------
void init_obj(void* self, Type type, flags fl) {
  Obj* obj    = self;
  obj->type   = type;
  obj->flags  = fl;

  register_obj(obj);
}

void mark_obj(void* self) {
  if (!self)
    return;

  if (has_flag(self, BLACK))
    return;

  set_flag(self, BLACK);

  if (mtable(self)->m.trace)
    objs_push(&Vm.heap.grays, self);

  else
    del_flag(self, GRAY);
}

void destruct_obj(void* self) {
  if (!self)
    return;

  usize freed = mtable(self)->size;

  if (mtable(self)->m.destruct)
     freed += mtable(self)->m.destruct(self);

  deallocate(self, freed, 1);
}

bool has_flag(void* self, flags fl) {
  return !!(((Obj*)self)->flags&fl);
}

bool set_flag(void* self, flags fl) {
  bool out = has_flag(self, fl);

  ((Obj*)self)->flags |= fl;

  return out;
}

bool del_flag(void* self, flags fl) {
  bool out = has_flag(self, fl);
  
  ((Obj*)self)->flags &= ~fl;

  return out;
}

// symbol api -----------------------------------------------------------------
static Sym** find_in_symbol_table(char* name) {
  assert(name != NULL);
  assert(*name != '\0');

  Sym** node = &SymbolTable;

  while (*node) {
    int o = strcmp(name, (*node)->name);

    if (o < 0)
      node = &(*node)->left;

    else if (o > 0)
      node = &(*node)->right;

    else
      break;
  }

  return node;
}



Sym* new_sym(bool genp, char* name) {
  assert(SymbolCounter < UINT32_MAX);
  assert(name != NULL && *name != '\0');

  Sym* out = mk_sym(name);
  init_sym(out, genp, name);

  return out;
}

void init_sym(Sym* self, flags fl, char* name) {
  assert(SymbolCounter < UINT32_MAX);
  assert(name != NULL && *name != '\0');

  fl |= LITERAL*(*name == ':');
  
  init_obj(self, SYM, fl);

  self->name     = (char*)&self[1];
  self->arity    = SymbolCounter++;
  self->hash     = mix_hashes(3, hash_str(name), hash_uint(self->arity), MetaTables[SYM].type_hash);
  self->bind     = UNDEFINED;
  self->left     = NULL;
  self->right    = NULL;

  set_flag(self, INITIALIZED);
}

Sym* get_sym(bool genp, char* name) {
  if (name == NULL || *name == '\0')
    name = "symbol";

  Sym* out;

  if (!genp) {
    Sym** node = find_in_symbol_table(name);

    if (*node == NULL)
      *node = new_sym(false, name);

    out = *node;
  } else {
    out = new_sym(true, name);
  }

  return out;
}

// func api -------------------------------------------------------------------

// initialization -------------------------------------------------------------
void object_init(void) {
  // initialize globals -------------------------------------------------------
  SymbolTable = NULL;

  register_obj(&EmptyList);
  register_obj(&EmptyString);
  register_obj(&EmptyTuple);
}
