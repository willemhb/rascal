#include "util/number.h"
#include "util/hashing.h"

#include "vm.h"
#include "equal.h"
#include "collection.h"
#include "type.h"
#include "object.h"

// generics
#include "tpl/describe.h"

// external APIs
// flags
bool getFl(void* p, flags_t f) {
  assert(p != NULL);
  Obj* o = p;
  return !!(o->flags & f);
}

bool setFl(void* p, flags_t f) {
  assert(p != NULL);
  Obj* o    = p;
  bool r    = !!(o->flags & f);
  o->flags |= f;

  return r;
}

bool delFl(void* p, flags_t f) {
  assert(p != NULL);
  Obj* o    = p;
  bool r    = !!(o->flags & f);
  o->flags &= ~f;

  return r;
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
  o->annot = &EmptyMap;
  o->flags = f;
  o->gray  = true;

  addToHeap(o);
}

// utility objects
// utility array types

// utility object types
TABLE_OBJ_API(Table, Value, Value, table, equalVal, hashVal, NOTHING, NOTHING);

// Table type
extern void   traceTable(void* p);

Vtable TableTable = {
  .valSize=sizeof(Table*),
  .objSize=sizeof(Table),
  .tag    =OBJ_TAG,
  .free   =freeTable,
  .trace  =traceTable
};

Type TableType = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&TableTable,
  .idno  =ALIST
};
