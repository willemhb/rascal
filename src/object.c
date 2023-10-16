#include "util/number.h"
#include "util/hashing.h"

#include "vm.h"
#include "equal.h"
#include "collection.h"
#include "type.h"
#include "object.h"

// generics
#include "tpl/describe.h"

ARRAY_API(Values, Value, values, Value, false);
ARRAY_API(Objects, Obj*, objects, Obj*, false);

// external APIs
// flags
int getFl(void* p, int f, int m) {
  assert(p != NULL);
  
  Obj* o = p;
  int out;

  if (m == 0)
    out = !!(o->flags & f);

  else if (f == 0)
    out = o->flags & m;

  else
    out = (o->flags & m) == f;

  return out;
}

int setFl(void* p, int f, int m) {
  assert(p != NULL);

  int out;
  Obj* o = p;

  if (m == 0) {
    out       = !!(o->flags & f);
    o->flags |= f;
  } else if (f == 0) {
    out       = o->flags & m;
    o->flags &= ~m;
  } else {
    out       = o->flags & m;
    o->flags &= ~m;
    o->flags |= f;
  }

  return out;
}

int delFl(void* p, int f) {
  return setFl(p, 0, f);
}

void* newObj(Type* t, int f, size_t e) {
  void* o;
  save(1, tag(t));
  o = allocate(&RlVm, t->vTable->objSize+e);
  initObj(o, t, f);
  unsave(1);
  return o;
}

void* cloneObj(void* p) {
  Obj* o;
  assert(p != NULL);
  save(1, tag(p));
  o = duplicate(&RlVm, p, sizeOf(p));
  addToHeap(o);
  unsave(1);
  return o;
}

void initObj(void* p, Type* t, int f) {
  Obj* o   = p;
  o->type  = t;
  o->annot = &emptyMap;
  o->flags = f;
  o->gray  = true;

  addToHeap(o);
}

// utility objects
ARRAY_OBJ_API(Binary8,  byte_t,   binary8,  int,      false);
ARRAY_OBJ_API(Binary16, uint16_t, binary16, int,      false);
ARRAY_OBJ_API(Binary32, uint32_t, binary32, uint32_t, false);
ARRAY_OBJ_API(Buffer8,  char,     buffer8,  int,      true);
ARRAY_OBJ_API(Buffer16, char16_t, buffer16, int,      true);
ARRAY_OBJ_API(Buffer32, char32_t, buffer32, int,      true);
ARRAY_OBJ_API(Alist,    Value,    alist,    Value,    false);

TABLE_OBJ_API(Table, Value, Value, table, equalVal, hashVal, NOTHING, NOTHING);
