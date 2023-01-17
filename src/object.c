#include <assert.h>

#include "type.h"
#include "object.h"
#include "runtime.h"

#include "util/collection.h"
#include "util/hashing.h"
#include "util/string.h"

#include "impl/alist.h"

ALIST(Objects, Object, padAlistSize);

// generic object API
Object createObject(RlType type, usize size) {
  assert(size >= BaseSize[type]);

  return allocate(size) + Offset[type];
}

void destroyObject(Object self) {
  bool allocated = objHead(self)->allocated;
  
  freeObject(self);

  if (allocated)
    deallocate(objStart(self), objHead(self)->size);
}



#include "impl/htable.h"
// symbol table
static Symbol makeSymbol(char *name, uhash hash) {
  static uint64 counter = 1;
  static usize  baseSize = sizeof(struct Symbol);

  usize nameLen  = strlen(name);
  usize objSize  = baseSize + nameLen + 1;
  
  assert(nameLen > 0);

  Object new           = createObject(SymbolType, objSize);
  
  initObject(new, SymbolType, objSize);
  
  OBJ_HASH(new)            = hash;
  objHead(new)->hashed     = true;
  SYM_HEAD(new)->idno      = counter++;
  SYM_HEAD(new)->bind      = NUL;
  SYM_HEAD(new)->isKeyWord = *name == ':';
  SYM_HEAD(new)->isBound   = false;

  memcpy(new, name, nameLen);

  return (Symbol)new;
}

static uhash hashSymbolTableKey(char *key) {
  return mixHashes(hashUlong(SymbolType), hashString(key));
}

static uhash reHashSymbolTableEntry(SymbolTableEntry entry) {
  return OBJ_HASH(AS_OBJ(entry.val));
}


static Symbol internInSymbolTable(SymbolTableEntry *entry, char *key, uhash hash) {
  return (entry->val = entry->key = makeSymbol(key, hash));
}


HTABLE(SymbolTable, char*, Symbol, streq, hashSymbolTableKey, reHashSymbolTableEntry, internInSymbolTable, NULL, NULL);


// symbol API
Symbol symbol(char *name) {
  return SymbolTableIntern(&RlSymbolTable, name);
}

// array objects
#include "impl/array.h"

ARRAY_OBJECT(Tuple, Value, padArraySize, 0);
ARRAY_OBJECT(String, Glyph, padStringSize, 1, '\0');

Tuple tuple(int nArgs, Value *args) {
  if (nArgs == 0)
    return EmptyTuple.array;

  Tuple out = createTuple(nArgs);

  initTuple(out, args);

  return out;
}

String string(char *chars) {
  usize s = strlen(chars);

  assert(s < INT32_MAX-1);

  int n = s;

  if (n == 0)
    return EmptyString.array;

  String out = createString(n);

  initString(out, chars);

  return out;
}
