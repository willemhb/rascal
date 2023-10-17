#ifndef rascal_vm_h
#define rascal_vm_h

#include "array.h"
#include "environment.h"
#include "runtime.h"

// C types
/* Composite global state object. */
struct Vm {
  // heap state
  struct {
    Obj*    objs;
    size_t  used;
    size_t  cap;
    Objects grays;
  } heap;

  // environment state
  struct {
    SymbolTable* symbols;
    NameSpace*   globals;
    Table*       annot;
    Table*       used;
  } envt;

  // reader state
  struct {
    ReadState  state;
    Stream*    source;
    Buffer8*   buffer;
    Table*     table;
    Alist      stack;
  } reader;

  // compiler state
  struct {
    Chunk*     chunk;
    Alist      stack;
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

// don't really fit anywhere else
extern struct Type PointerType, FuncPtrType;

// external API
void initVm(Vm* vm);
void freeVm(Vm* vm);
void syncVm(Vm* vm);

size_t push(Value x);
Value  pop(void);
size_t pushn(size_t n);
Value  popn(size_t n);
Value* peek(int i);
size_t save(size_t n, ...);
void   unsave(size_t n);

#endif
