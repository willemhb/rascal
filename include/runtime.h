#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "common.h"
#include "value.h"

// C types
struct Context {
  struct Context* next;

  // Rascal execution state
  Value* sp, *ap;
  uint16_t* ip;
  Closure* code;

  // C execution state
  jmp_buf buf;
} ErrorContext;

// external API
void initContext(Context* ctx);
void freeContext(Context* ctx);
void resetContext(Context* ctx);

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
