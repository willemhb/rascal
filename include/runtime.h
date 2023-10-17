#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "object.h"

// C types
/* a single stack frame. */
struct Frame {
  Closure*  code;
  uint16_t* ip;
  size_t    bp;
};

struct Control {
  Obj      obj;
  Frame    frame;
  Value*   stackCopy;
  Frame*   framesCopy;
  size_t   nStack, nFrames;
};

struct UpValue {
  Obj      obj;
  UpValue* next;
  size_t   offset;
  Value    value;
};

struct Context {
  Context* next;

  // Rascal execution state
  size_t sp, fp, bp;
  uint16_t* ip;
  Closure* code;

  // C execution state
  jmp_buf buf;
} ErrorContext;

// external API
void initRuntime(Vm* vm);
void freeRuntime(Vm* vm);
void resetRuntime(Vm* vm);

void saveState(Vm* vm, Context* ctx);
void restoreState(Vm* vm, Context* ctx);

void    raise(const char* fname, const char* fmt, ...);
bool    require(bool test, const char* fname, const char* fmt, ...);
size_t  argco(size_t expected, size_t got, const char* fname);
size_t  vargco(size_t expected, size_t got, const char* fname);
Type*   argtype(Type* expected, Value got, const char* fname);
Type*   argtypes(size_t n, Value got, const char* fname, ...);
size_t  argcos(size_t n, size_t got, const char* fname, ...);
size_t  vargcos(size_t n, size_t got, const char* fname, ...);

#define try                                                             \
  ErrorContext _ctx; int l__tr, l__ca;                                  \
  saveState(&RlVm, &_ctx); RlVm.context.ctx = &_ctx;                    \
  if (!setjmp(_ctx.buf))                                                \
    for (l__tr=1; l__tr; l__tr=0, (void)(RlVm.context.ctx=_ctx.next))

#define catch                                                   \
  else                                                          \
    for (l__ca=1; l__ca; l__ca=0, restoreState(&RlVm, &_ctx))

#endif
