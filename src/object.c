#include <assert.h>

#include "object.h"
#include "runtime.h"

#include "util/collection.h"
#include "util/hashing.h"
#include "util/string.h"

#include "impl/alist.h"

ALIST(Objects, Object, padAlistSize);

#include "impl/htable.h"

// symbol table
static Symbol makeSymbol(char *name, uhash hash) {
  static uint64 counter = 1;
  static usize  baseSize = sizeof(struct Symbol);

  usize nameLen  = strlen(name);
  usize objSize  = baseSize + nameLen + 1;
  
  assert(nameLen > 0);

  Object new           = createObject(SymbolType, objSize);
  struct Object *head  = objHead(new);
  struct Symbol *sHead = (struct Symbol*)objStart(new);
  Symbol sym           = (Symbol)new;
  
  initObject(new, SymbolType, objSize);
  
  head->hash    = hash;
  head->hashed  = true;
  sHead->idno   = counter++;

  memcpy(sym, name, nameLen);

  return sym;
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


HTABLE(SymbolTable, char*, Symbol, streq, hashSymbolTableKey, reHashSymbolTableEntry,
       internInSymbolTable,
       NULL,
       NULL);
