#include "util/hashing.h"

#include "runtime.h"
#include "memory.h"
#include "environment.h"
#include "read.h"
#include "compile.h"
#include "eval.h"
#include "vm.h"
#include "equal.h"

// generics
#include "tpl/describe.h"

ARRAY_TYPE(TextBuffer, char, int, true);

bool compareReadTableKeys(int cx, int cy) {
  return cx == cy;
}

void internReadTableKey(ReadTable* table, ReadTableEntry* entry, int key, ReadFn* value) {
  (void)table;
  entry->key = key;
  entry->val = *value;
}

uint64_t hashCharacter(int ch) {
  return ch;
}

TABLE_TYPE(ReadTable,
           readTable,
           int,
           ReadFn,
           compareReadTableKeys,
           hashCharacter,
           internReadTableKey,
           '\0',
           NULL);

bool compareSymbolCacheKeys(char* xs, char* ys) {
  return strcmp(xs, ys) == 0;
}

void internSymbolCacheKey(SymbolCache* table, SymbolCacheEntry* entry, char* key, Symbol** value) {
  (void)table;

  Symbol* atom = newSymbol(key, true);
  entry->key   = atom->name;
  entry->val   = atom;
  *value       = atom;
}

TABLE_TYPE(SymbolCache,
           symbolCache,
           char*,
           Symbol*,
           compareSymbolCacheKeys,
           hashString,
           internSymbolCacheKey,
           NULL,
           NULL);

bool compareLoadCacheKeys(Bits* x, Bits* y) {
  return strcmp(x->data, y->data) == 0;
}

void internLoadCacheKey(LoadCache* table, LoadCacheEntry* entry, Bits* key, Value* value) {
  (void)table;

  entry->key = key;
  entry->val = *value;
}

TABLE_TYPE(LoadCache,
           loadCache,
           Bits*,
           Value,
           compareLoadCacheKeys,
           hashObj,
           internLoadCacheKey,
           NULL,
           NOTHING);

bool compareAnnotationsKeys(Value x, Value y) {
  return x == y;
}

uint64_t hashAnnotationsKey(Value x) {
  return hashWord(x);
}

void internAnnotationsKey(Annotations* table, AnnotationsEntry* entry, Value key, Map** value) {
  (void)table;
  entry->key = key;
  entry->val = *value = &emptyMap;
}

TABLE_TYPE(Annotations,
           annotations,
           Value,
           Map*,
           compareAnnotationsKeys,
           hashAnnotationsKey,
           internAnnotationsKey,
           NOTHING,
           NULL);

// external API
void initVm(Vm* vm) {
  initHeap(vm);
  initEnvt(vm);
  initReader(vm);
  initCompiler(vm);
  initInterpreter(vm);
  initRuntime(vm);
}

void freeVm(Vm* vm) {
  freeRuntime(vm);
  freeInterpreter(vm);
  freeCompiler(vm);
  freeReader(vm);
  freeEnvt(vm);
  freeHeap(vm);
}

void syncVm(Vm* vm) {
  syncReader(vm);
  syncCompiler(vm);
  syncInterpreter(vm);
}

size_t push(Value x) {
  assert(RlVm.exec.sp < N_STACK);
  size_t out = RlVm.exec.sp;
  RlVm.stackBase[RlVm.exec.sp++] = x;
  return out;
}

Value pop(void) {
  assert(RlVm.exec.sp > 0);
  return RlVm.stackBase[--RlVm.exec.sp];
}

size_t pushn(size_t n) {
  assert(RlVm.exec.sp + n < N_STACK);
  size_t out = RlVm.exec.sp;
  RlVm.exec.sp += n;

  return out;
}

Value popn(size_t n) {
  assert(n <= RlVm.exec.sp);
  Value out = RlVm.stackBase[RlVm.exec.sp-n];
  RlVm.exec.sp -= n;
  return out;
}

Value* peek(int i) {
  if (i < 0)
    i += RlVm.exec.sp;

  assert(i >= 0 && (size_t)i < RlVm.exec.sp);

  return RlVm.stackBase+i;
}

void save(size_t n, ...) {
  Value buf[n];
  va_list va;
  va_start(va, n);

  for (size_t i=0; i<n; i++)
    buf[i] = va_arg(va, Value);

  va_end(va);

  nWriteValues(&RlVm.heap.saved, n, buf);
}

void unsave(size_t n) {
  assert(n <= RlVm.heap.saved.count);
  nPopValues(&RlVm.heap.saved, n);
}
