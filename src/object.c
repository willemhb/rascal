#include <string.h>

#include "util/hashing.h"

#include "vm.h"
#include "object.h"

// template implementations
#include "tpl/describe.h"

ARRAY_TYPE(Objects, Obj*, Obj*, false);
ARRAY_TYPE(ByteCode, uint16_t, int, false);

bool compareSymbolTableKeys(char* xs, char* ys) {
  return strcmp(xs, ys) == 0;
}

void internSymbolTableKey(SymbolTable* table, SymbolTableEntry* entry, char* key, Symbol** value) {
  (void)table;
  Symbol* atom   = newSymbol(key);
  entry->key   = atom->name;
  entry->val   = atom;
  *value       = atom;
}

TABLE_TYPE(SymbolTable,
           symbolTable,
           char*,
           Symbol*,
           compareSymbolTableKeys,
           hashString,
           internSymbolTableKey,
           NULL,
           NULL);

bool compareNameSpaceKeys(Symbol* x, Symbol* y) {
  return x == y;
}

void internNameSpaceKey(NameSpace* table, NameSpaceEntry* entry, Symbol* key, size_t* val) {
  entry->key = key;
  entry->val = *val = table->count;
}

TABLE_TYPE(NameSpace,
           nameSpace,
           Symbol*,
           Value,
           compareNameSpaceKeys,
           hashObject,
           internNameSpaceKey,
           NULL,
           UINT64_MAX);

// internal forward declarations
static void  initObject(void* pointer, Type type, int flags) {
  Obj* obj  = pointer;

  obj->type        = type;
  obj->gray        = true;
  obj->flags       = flags;
  obj->next        = vm.heap.objects;
  vm.heap.objects = obj;
}

static void* newObject(Type type) {
  Obj* out  = allocate(sizeOfType(type), true);

  initObject(out, type, 0);

  return out;
}

// miscellaneous utilities & helpers
static void hashObjectIdentity(Obj* obj) {
  obj->hash   = hashPtr(obj);
  obj->hashed = true;
}

static void hashSymbol(Symbol* symbol) {
  uint64_t idHash     = hashWord(symbol->idno);
  uint64_t nameHash   = hashString(symbol->name);
  uint64_t symbolHash = mixHashes(idHash, nameHash);
  symbol->obj.hash    = symbolHash;
  symbol->obj.hashed  = true;
}

static void hashBits(Bits* bits) {
  bits->obj.hash   = hashBytes(bits->data, bits->arity * bits->elSize);
  bits->obj.hashed = true;
}

static void hashList(List* list) {
  List* curr=list,* prev=&emptyList,* tmp;

  while (!curr->obj.hashed) {
    // Traverse until we find a hashed node. Save parents by reversing list.
    // Empty list is hashed at startup, so this always terminates.
    tmp        = prev;
    prev       = curr;
    curr       = curr->tail;
    prev->tail = tmp;
  }

  while (prev != &emptyList) {
    uint64_t headHash = hashValue(prev->head);
    uint64_t tailHash = curr->obj.hash;
    prev->obj.hash    = mixHashes(headHash, tailHash);
    prev->obj.hashed  = true;
    tmp               = prev->tail;
    prev->tail        = curr;
    curr              = prev;
    prev              = tmp;
  }
}

static bool equalBits(Bits* xs, Bits* ys) {
  return
    xs->arity  == ys->arity  &&
    xs->elSize == ys->elSize &&
    memcmp(xs->data,
           ys->data,
           xs->arity*xs->elSize) == 0;
}

static bool equalLists(List* xs, List* ys) {
  bool out = xs->arity == ys->arity;

  for (; out && xs->arity; xs=xs->tail, ys=ys->tail )
    out = equalValues(xs->head, ys->head);

  return out;
}

uint64_t hashObject(void* ob) {
  assert(ob != NULL);

  Obj* obj = ob;

  if (!obj->hashed)
    switch(obj->type) {
      case SYMBOL: hashSymbol(ob);          break;
      case BITS:   hashBits(ob);            break;
      case LIST:   hashList(ob);            break;
      default:     hashObjectIdentity(obj); break;
  }

  return mixHashes(hashType(obj->type), obj->hash);
}

bool equalObjects(void* obx, void* oby) {
  assert(oby != NULL);
  assert(obx != NULL);

  bool out = obx == oby;

  if (out == false) {
    Type tx = objectType(obx), ty = objectType(oby);

    if (tx == ty)
      switch (tx) {
        case BITS:  out = equalBits(obx, oby);   break;
        case LIST:  out = equalLists(obx, oby);  break;
        default:                                 break;
      }
  }

  return out;
}

void freeObject(void* ob) {
  (void)ob;
}

// constructors
// symbol constructors
Symbol* newSymbol(char* name) {
  size_t n = strlen(name)+1;

  char* copy = duplicate(name, n, false);
  Symbol* out  = newObject(SYMBOL);

  out->name  = copy;
  out->idno  = ++vm.environment.symbolCounter;

  return out;
}

Symbol* getSymbol(char* buffer) {
  return internSymbol(&vm.environment, buffer);
}

// chunk constructors
Chunk* newChunk(void) {
  Chunk* out = newObject(CHUNK);

  initValues(&out->vals);
  initByteCode(&out->code);

  return out;
}

// bits constructors
static void initBits(Bits* bits, void* data, size_t count, size_t elSize) {
  bits->data       = data;
  bits->arity      = count;
  bits->elSize     = elSize;
}

Bits* newBits(void* data, size_t count, size_t elSize, Encoding encoding) {
  Bits* out = newObject(BITS);
  void* spc = allocate((count + !!encoding) * elSize, false);

  memcpy(spc, data, count * elSize);
  initObject(out, BITS, encoding);
  initBits(out, spc, count, elSize);

  return out;
}

Bits* newString(char* data, size_t count) {
  return newBits(data, count, sizeof(char), ASCII);
}

// list constructors
static void initList(List* node, Value head, List* tail) {
  node->head  = head;
  node->tail  = tail;
  node->arity = tail->arity+1;
}

List* newList(Value head, List* tail) {
  List* node = newObject(LIST);
  initList(node, head, tail);
  return node;
}

List* newList1(Value head) {
  return newList(head, &emptyList);
}

List* newList2(Value arg1, Value arg2) {
  List* tail = newList1(arg2);
  return newList(arg1, tail);
}

List* newListN(size_t n, Value* args) {
  List* out;

  if (n == 0)
    out = &emptyList;

  else if (n == 1)
    out = newList1(args[0]);

  else if (n == 2)
    out = newList2(args[0], args[1]);

  else {
    out = allocate(sizeof(List) * n, true);

    for (size_t i=n; i>0; i--) {
      initObject(&out[i-1], LIST, 0);
      initList(&out[i-1], args[i-1], i == n ? &emptyList : &out[i]);
    }
  }

  return out;
}

Value mkListN(size_t n, Value* args) {
  return TAG_OBJ(newListN(n, args));
}
