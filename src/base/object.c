#include "base/object.h"

#include "runtime/memory.h"


/* API */
bool is_obj(Val x) {
  return TAG_BITS(x) == OBJ_TAG;
}

Obj* as_obj(Val x) {
  return (Obj*)VAL_BITS(x);
}

Val mk_obj(Obj* o) {
  return (((ValData)o).as_val & VAL_MASK) | OBJ_TAG;
}

// flag getters & setters -----------------------------------------------------
bool has_flag(Obj* o, int fl) {
  return !!(o->flags&fl);
}

bool set_flag(Obj* o, int fl) {
  bool out = !has_flag(o, fl);
  o->flags |= fl;
  return out;
}

bool clear_flag(Obj* o, int fl) {
  bool out  = has_flag(o, fl);
  o->flags &= ~fl;
  return out;
}

bool has_wflag(Obj* o, int fl, int m) {
  return (o->flags&m) == fl;
}

int get_flags(Obj* o, int m) {
  return o->flags&m;
}

int set_flags(Obj* o, int fl, int m) {
  int out   = o->flags & m;
  o->flags &= ~m;
  o->flags |= fl;

  return out;
}

int clear_flags(Obj* o, int m) {
  int out   = o->flags & m;
  o->flags &= ~m;

  return out;
}

// lifetime & memory management -----------------------------------------------
void init_obj(Obj* self, Type type, int fl) {
  self->next  = Heap.live;
  Heap.live   = self;
  self->type  = type;
  self->flags = 0;
  self->hash  = 0;
  set_flags(self, fl|GRAY, 0x3ff);
}

void mark_obj(Obj* self) {
  if (!self)
    return;

  if (has_flag(self, BLACK))
    return;

  set_flag(self, BLACK);

  if (mtable(self)->trace)
    add_gray(self);

  else
    clear_flag(self, GRAY);
}

void destruct_obj(Obj* self) {
  usize freed = mtable(self)->size;

  if (mtable(self)->destruct)
     freed += mtable(self)->destruct(self);

  if (!has_flag(self, NOFREE))
    deallocate(self, freed);
}
