#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "object.h"

// C types
/* a single stack frame. */
struct Frame {
  Closure*  code;
  uint16_t* ip;
  size_t    bp;
  size_t    pp;
};

struct Control {
  Obj      obj;
  Frame    frame;
  Value*   stack_copy;
  Frame*   frames_copy;
  size_t   n_stack, n_frames;
};

struct UpValue {
  Obj      obj;
  UpValue* next;
  size_t   offset;
  Value    value;
};

struct Context {
  Context* next;

  // GC state
  GcFrame* frames;

  // Rascal execution state
  size_t sp, fp, bp, pp;
  uint16_t* ip;
  Closure* code;

  // C execution state
  jmp_buf buf;
};

// external API
void init_runtime(Vm* vm);
void free_runtime(Vm* vm);
void reset_runtime(Vm* vm);

void save_state(Vm* vm, Context* ctx);
void restore_state(Vm* vm, Context* ctx);

void    raise(const char* fname, const char* fmt, ...);
bool    require(bool test, const char* fname, const char* fmt, ...);
size_t  argco(size_t expected, size_t got, const char* fname);
size_t  vargco(size_t expected, size_t got, const char* fname);
Type*   argtype(Type* expected, Value got, const char* fname);
Type*   argtypes(size_t n, Value got, const char* fname, ...);
size_t  argcos(size_t n, size_t got, const char* fname, ...);
size_t  vargcos(size_t n, size_t got, const char* fname, ...);

#define try                                                             \
  Context _ctx; int l__tr, l__ca;                                       \
  save_state(&RlVm, &_ctx); RlVm.ctx = &_ctx;                           \
  if (!setjmp(_ctx.buf))                                                \
    for (l__tr=1; l__tr; l__tr=0, (void)(RlVm.ctx=_ctx.next))

#define catch                                                   \
  else                                                          \
    for (l__ca=1; l__ca; l__ca=0, restore_state(&RlVm, &_ctx))

#endif
