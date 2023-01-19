#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "type.h"
#include "object.h"
#include "runtime.h"

#include "util/collection.h"
#include "util/hashing.h"
#include "util/string.h"

#include "impl/alist.h"

ALIST(Objects, Object, padAlistSize);

// generic object API
Object constructObject(ObjectInit *args) {
  Object new = createObject(args);

  initObject(new, args);

  return new;
}

Object createObject(ObjectInit *args) {
  // default parameters (Alloc method should override if different)
  args->size      = BaseSize[args->type];
  args->offset    = Offset[args->type];
  args->lendee    = false;
  args->inlined   = true;
  args->allocated = true;

  struct Object *newObj;
  
  if (Alloc[args->type])
    newObj = Alloc[args->type](args);

  else
    newObj = allocate(BaseSize[args->type]);

  return newObj->space;
}

void destroyObject(Object self, struct Object **next) {
  freeObject(self, next);

  if (OBJ_HEAD(self)->allocated)
    deallocate(OBJ_START(self), OBJ_HEAD(self)->size);
}

void initObject(Object self, ObjectInit *args) {
  Init[args->type](self, args);
}

void freeObject(Object self, struct Object **next) {
  struct Object *head = OBJ_HEAD(self);
  RlType type         = head->type;
  
  if (next)
    *next = head->next;

  if (Free[type])
    Free[type](self);
}

// symbol & symbol table API
#include "impl/htable.h"
void *allocSymbol(ObjectInit *args) {
  static uint64 counter  = 1;
  char  *name            = args->SymbolInit.name;

  // invalid symbol names
  assert(name);
  assert(*name != '\0');
  
  int    nArgs           = strlen(name);

  args->SymbolInit.nArgs = nArgs;
  args->SymbolInit.idno  = counter++;
  args->size            += nArgs + 1;

  return allocate(args->size);
}

void initSymbol(void *self, ObjectInit *args) {
  INIT_OBJ(self, args);
  memcpy(self, args->SymbolInit.name, args->SymbolInit.nArgs);

  struct Symbol *sym = self - sizeof(struct Symbol);
  sym->idno          = args->SymbolInit.idno;
  sym->bind          = NUL;
  sym->obj.flags    |= (*(char*)self == ':') * LiteralSymbol;
}

static Symbol makeSymbol(char *name, uhash hash) {
  ObjectInit args = {
    .type  = SymbolType,
    .hash  = hash,
    .SymbolInit = { .name=name }
  };

  return (Symbol)constructObject(&args);
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

Value  symbolToValue(Symbol s) {
  return objectToValue((Object)s);
}

Symbol symbol(char *name) {
  return SymbolTableIntern(&RlSymbolTable, name);
}

// function & function API
void initFunction(void *self, ObjectInit *args) {
  INIT_OBJ(self, args);

  ((Function)self)->name    = args->FunctionInit.name;
  ((Function)self)->type    = args->FunctionInit.type;
  ((Function)self)->vMethod = NULL;

  initObjects(&((Function)self)->methods);
}

Value    functionToValue(Function f) {
  return objectToValue((Object)f);
}

Function function(char *name, RlType type) {
  ObjectInit args = {
    .type=FunctionType,
    .FunctionInit = { symbol(name), type }
  };

  return (Function)constructObject(&args);
}

// list & list API
struct ListObject {
  struct Object obj;
  struct List   list;
};

struct ListObject EmptyList = {
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

void *allocList(ObjectInit *init) {
  usize objSize = BaseSize[init->type];
  int   nArgs   = init->ListInit.nArgs;

  assert(nArgs > 0); // invalid length (EmptyList should have already been returned).

  return allocArr(nArgs, objSize);
}

void initList(void *self, ObjectInit *init) {
  struct ListObject *xs = self - sizeof(struct Object);
  int    nArgs          = init->ListInit.nArgs;
  Value *args           = init->ListInit.args;

  for (int i = 0; i < nArgs; i++) {
    INIT_OBJ(&xs->obj, init);

    int len = nArgs - i;

    xs->list.length = len;
    xs->list.head   = args[i];

    if (len > 1)
      xs->list.tail = &(xs+1)->list;

    else
      xs->list.tail = &EmptyList.list;

    xs++;
  }
}

Value listToValue(List l) {
  return objectToValue((Object)l);
}

List list(Value *args, int nArgs) {
  if (nArgs == 0)
    return &EmptyList.list;

  ObjectInit objArgs = {
    .type=ListType,
    .ListInit = { args, nArgs }
  };

  return (List)constructObject(&objArgs);
}

// pair & pair API
void  initPair(void *self, ObjectInit *args) {
  INIT_OBJ(self, args);

  ((Pair)self)->car = args->PairInit.car;
  ((Pair)self)->cdr = args->PairInit.cdr;
}

Value pairToValue(Pair p) {
  return objectToValue((Object)p);
}

Pair pair(Value car, Value cdr) {
  ObjectInit args = {
    .type=PairType,
    .PairInit = { car, cdr }
  };

  return (Pair)constructObject(&args);
}

// tuple & tuple API
#include "impl/array.h"
ARRAY_OBJECT(Tuple, Value, padArraySize, 0);

Value tupleToValue(Tuple t) {
  return objectToValue((Object)t);
}

Tuple tuple(Value *args, int nArgs) {
  if (nArgs == 0)
    return EmptyTuple.array;

  ObjectInit objArgs = {
    .type=TupleType,
    .TupleInit = { args, nArgs }
  };

  return (Tuple)constructObject(&objArgs);
}

// string & string API
ARRAY_OBJECT(String, Glyph, padStringSize, 1, '\0');

Value stringToValue(String s) {
  return objectToValue((Object)s);
}

String string(char *chars) {
  assert(chars);

  if (*chars == '\0')
    return EmptyString.array;

  ObjectInit args = {
    .type=StringType,
    .StringInit = { chars, strlen(chars) }
  };

  return (String)constructObject(&args);
}

// Internal objects
// bytecode & bytecode API
ARRAY_OBJECT(ByteCode, uint16, padArraySize, 0);

Value byteCodeToValue(ByteCode b) {
  return objectToValue((Object)b);
}

ByteCode bytecode(uint16 *code, int nArgs) {
  assert(nArgs > 0);

  ObjectInit args = {
    .type=ByteCodeType,
    .ByteCodeInit = { code, nArgs }
  };

  return (ByteCode)constructObject(&args);
}

// namespc & namespc API
void initNameSpc(void *self, )
