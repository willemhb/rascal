#ifndef rascal_vm_h
#define rascal_vm_h

#include "common.h"
#include "value.h"
#include "object.h"

// generics
#include "tpl/declare.h"

ARRAY_TYPE(TextBuffer, char);
TABLE_TYPE(ReadTable, readTable, int, ReadFn);
TABLE_TYPE(SymbolCache, symbolCache, char*, Symbol*);
TABLE_TYPE(LoadCache, loadCache, Bits*, Value);
TABLE_TYPE(Annotations, annotations, Value, Map*);

/* Composite global state object. */
struct Vm {
  // heap state
  struct {
    Obj*    objs;
    size_t  used;
    size_t  cap;
    Objects grays;
    Values  saved;
  } heap;

  // environment state
  struct {
    uintptr_t   nSymbols;
    SymbolCache symbols;
    Annotations annot;
    NsMap       globals;
    LoadCache   used;
  } toplevel;

  // reader state
  struct {
    FILE*      source;
    ReadState  state;
    TextBuffer buffer;
    ReadTable  table;
    Values     stack;
  } reader;

  // compiler state
  struct {
    Chunk*     chunk;
    NameSpace* ns;
    Values     stack;
  } compiler;

  // execution state
  struct {
    size_t sp, fp, bp;
    UpValue*  upVals;
    Closure*  code;
    uint16_t* ip;
  } exec;

  // error context
  Context* ctx;

  // miscellaneous state
  Value* stackBase, * stackEnd;
  Frame* framesBase, * framesEnd;
};

// globals
#define N_STACK 32768
#define N_FRAME 8192

extern Vm RlVm;
extern struct Frame Frames[N_FRAME];
extern Value Stack[N_STACK];

// external API
void initVm(Vm* vm);
void freeVm(Vm* vm);
void syncVm(Vm* vm);

size_t push(Value x);
Value  pop(void);
size_t pushn(size_t n);
Value  popn(size_t n);
Value* peek(int i);
void   save(size_t n, ...);
void   unsave(size_t n);
 
#endif
