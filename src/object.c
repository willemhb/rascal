#include <string.h>

#include "hashing.h"
#include "object.h"

// template implementations
#include "describe.h"

ARRAY_TYPE(Objects, Obj*);
ARRAY_TYPE(ByteCode, uint16_t);

bool compareSymbolTableKeys(char* xs, char* ys) {
  return strcmp(xs, ys) == 0;
}

void internSymbolTableKey(SymbolTableEntry* entry, char* key, Atom** value) {
  Atom* atom   = newAtom(key);
  entry->key   = atom->name;
  entry->val   = atom;
  *value       = atom;
}

TABLE_TYPE(SymbolTable,
           symbolTable,
           char*,
           Atom*,
           compareSymbolTableKeys,
           hashString,
           internSymbolTableKey,
           NULL,
           NULL);
