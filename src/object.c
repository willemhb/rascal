#include <assert.h>
#include <stdarg.h>

#include "type.h"
#include "object.h"
#include "runtime.h"

#include "util/collection.h"
#include "util/hashing.h"
#include "util/string.h"

#include "impl/alist.h"

ALIST(Objects, Object, padAlistSize);

// generic object API
Object constructObject(RlType type, void *args) {
  Object new = createObject(type, args);

  initObject(new, type, args);

  return new;
}

Object createObject(RlType type, void *args) {
  void *spc;
  usize size, offset = Offset[type];

  if (Alloc[type]) // NB: method responsible for initializing non-standard size/arity information
    size = Alloc[type](type, args, &spc);

  else {
    size = BaseSize[type];
    spc  = allocate(size);
  }

  struct Object *head = spc + offset - sizeof(struct Object);

  // initialize standard size size & arity information (plus a few other fields)
  head->next      = NULL;
  head->type      = type;
  head->size      = size;
  head->offset    = offset;
  head->allocated = true;
  head->gray      = true;
  head->black     = false;

  return head->space;
}

void destroyObject(Object self, struct Object **next) {
  bool allocated = objHead(self)->allocated;

  freeObject(self, next);

  if (allocated)
    deallocate(objStart(self), OBJ_HEAD(self)->size);
}

void initObject(Object self, RlType type, void *args) {
  OBJ_HEAD(self)->hash      = 0;
  OBJ_HEAD(self)->flags     = 0;
  OBJ_HEAD(self)->hashed    = false;

  if (Init[type])
    Init[type](self, type, args);
}

void freeObject(Object self, struct Object **next) {
  struct Object *head = OBJ_HEAD(self);
  RlType type         = head->type;
  
  if (next)
    *next = head->next;

  if (Free[type]) // NB: method responsible for updating deallocate information
    Free[type](self);
}

// symbol & symbol table API
#include "impl/htable.h"
struct SymbolArgs {
  char *name;
  usize nameLen;
  uhash hash;
};

usize allocSymbol(RlType type, void *args, void **dst) {
  static uint64 counter    = 1;
  struct SymbolArgs *sArgs = args;

  usize total = BaseSize[type] + sArgs->nameLen + 1;
  *dst        = allocate(total);

  struct Symbol *sym = *dst;

  sym->obj.inlined = true;
  sym->idno        = counter++;

  return total;
}

void initSymbol(void *self, RlType type, void *args) {
  (void)type;

  struct SymbolArgs *sArgs = args;
  struct Symbol *symSelf   = SYM_HEAD(self);

  symSelf->obj.hash   = sArgs->hash;
  symSelf->obj.hashed = true;
  symSelf->bind       = NUL;
  
  memcpy(self, sArgs->name, sArgs->nameLen);
}

static Symbol makeSymbol(char *name, uhash hash) {
  assert(name != NULL);
  assert(*name != '\0');

  struct SymbolArgs args = { name, strlen(name), hash };

  return (Symbol)constructObject(SymbolType, &args);
}

static uhash hashSymbolTableKey(char *key) {
  return mixHashes(hashUlong(SymbolType), hashString(key));
}

static uhash reHashSymbolTableEntry(SymbolTableEntry entry) {
  return OBJ_HEAD(entry.val)->hash;
}

static Symbol internInSymbolTable(SymbolTableEntry *entry, char *key, uhash hash) {
  return (entry->val = entry->key = makeSymbol(key, hash));
}

HTABLE(SymbolTable, char*, Symbol, streq, hashSymbolTableKey, reHashSymbolTableEntry, internInSymbolTable, NULL, NULL);

Symbol symbol(char *name) {
  return SymbolTableIntern(&RlSymbolTable, name);
}

// list & list API
struct {
  struct Object obj;
  struct List list;
} EmptyList = {
  .obj={
    .next=NULL,
    .hash=0,
    .type=ListType,
    .flags=0,
    .hashed=false,
    .lendee=false,
    .inlined=true,
    .allocated=false,
    .gray=false,
    .black=true,
    .offset=sizeof(struct Object),
    .size=sizeof(EmptyList)
  },

  .list={
    .tail  =&EmptyList.list,
    .head  =NUL,
    .length=0
  }
};

usize allocList(RlType type, void *args, void **dst) {
  usize total = ;
}

// pair & pair API
struct 

Pair pair(Value car, Value cdr) {
  
}

// tuple & tuple API
#include "impl/array.h"

ARRAY_OBJECT(Tuple, Value, padArraySize, 0);

Tuple tuple(Value *args, int nArgs) {
  if (nArgs == 0)
    return EmptyTuple.array;

  struct TupleArgs tArgs = { nArgs, nArgs, args };

  return (Tuple)constructObject(TupleType, &tArgs);
}

// string & string API
ARRAY_OBJECT(String, Glyph, padStringSize, 1, '\0');
