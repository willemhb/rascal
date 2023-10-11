#include "opcodes.h"
#include "vm.h"

// external API
Heap* heap(Vm* vm) {
  return &vm->heap;
}

Obj* heapObjects(Vm* vm) {
  return heap(vm)->objects;
}

size_t heapUsed(Vm* vm) {
  return heap(vm)->used;
}

size_t heapCapacity(Vm* vm) {
  return heap(vm)->capacity;
}

Objects* heapGrays(Vm* vm) {
  return &heap(vm)->grays;
}

Context* context(Vm* vm) {
  return &vm->context;
}

bool panicking(Vm* vm) {
  return context(vm)->panicking;
}

Environment* environment(Vm* vm) {
  return &vm->environment;
}

size_t nSymbols(Vm* vm) {
  return environment(vm)->nSymbols;
}

SymbolTable* symbols(Vm* vm) {
  return &environment(vm)->symbols;
}

NameSpace* globalNs(Vm* vm) {
  return &environment(vm)->globalNs;
}

Values* globalVals(Vm* vm) {
  return &environment(vm)->globalVals;
}

Reader* reader(Vm* vm) {
  return &vm->reader;
}

FILE* source(Vm* vm) {
  return reader(vm)->source;
}

ReaderState readState(Vm* vm) {
  return reader(vm)->state;
}

TextBuffer* readBuffer(Vm* vm) {
  return &reader(vm)->buffer;
}

char* token(Vm* vm) {
  return readBuffer(vm)->data;
}

ReadTable* readTable(Vm* vm) {
  return &reader(vm)->table;
}

Values* readStack(Vm* vm) {
  return &reader(vm)->stack;
}

Compiler* compiler(Vm* vm) {
  return &vm->compiler;
}

Chunk* compilerChunk(Vm* vm) {
  return compiler(vm)->chunk;
}

Values* compilerStack(Vm* vm) {
  return &compiler(vm)->stack;
}

Interpreter* interpreter(Vm* vm) {
  return &vm->interpreter;
}

Value* sp(Vm* vm) {
  return interpreter(vm)->sp;
}

Value* vp(Vm* vm) {
  return interpreter(vm)->vp;
}

Value* ep(Vm* vm) {
  return interpreter(vm)->ep;
}

Chunk* code(Vm* vm) {
  return interpreter(vm)->code;
}

void initVm(Vm* vm) {
  initHeap(heap(vm));
  initContext(context(vm));
  initEnvironment(environment(vm));
  initReader(reader(vm));
  initCompiler(compiler(vm));
  initInterpreter(interpreter(vm), TheStack, N_STACK);
}

void freeVm(Vm* vm) {
  freeHeap(heap(vm));
  freeContext(context(vm));
  freeEnvironment(environment(vm));
  freeReader(reader(vm));
  freeCompiler(compiler(vm));
  freeInterpreter(interpreter(vm));
}

void syncVm(Vm* vm) {
  syncReader(reader(vm));
  syncCompiler(compiler(vm));
  syncInterpreter(interpreter(vm));
}
