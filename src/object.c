#include <string.h>

#include "hashing.h"
#include "vm.h"
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

// internal forward declarations
static void* newObject(Type type, size_t extra);
static void  initObject(void* pointer, Type type);

// constructors
// atom
Atom* newAtom(char* name) {
  size_t n = strlen(name)+1;

  char* copy = duplicate(name, n, false);
  Atom* out  = newObject(ATOM, 0);

  out->name  = copy;
  out->idno  = ++vm.symbolCounter;

  return out;
}

Atom* getAtom(char* name, size_t tokSize) {
  if (tokSize == 0)
    tokSize = strlen(name);

  // copy string in case it comes from non-null terminated token.
  char buffer[tokSize+1]; Atom* out = NULL;
  strncpy(buffer, name, tokSize);
  buffer[tokSize] = '\0';

  symbolTableAdd(&vm.symbolTable, buffer, &out);

  return out;
}
