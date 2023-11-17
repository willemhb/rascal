#include "vm/memory.h"
#include "vm/error.h"
#include "vm/compile.h"

/* Globals */
#define N_VALUES 65536
#define N_FRAMES 16384

/* Value stack and call stack. */
Value     CompilerValues[N_VALUES];
CompFrame CompilerFrames[N_FRAMES];

/* Internal API */
static void mark_compiler_frame(CompFrame* f) {
  mark(f->code);
  mark(f->vals);
  mark(f->envt);
}

static void mark_compiler_frames(void) {
  for (size_t i=0; i<Ctx.i.fp; i++)
    mark_compiler_frame(&CompilerFrames[i]);
}

/* External API */
Value* peek_compiler_val(int n) {
  if (n < 0)
    n += Ctx.c.sp;
  
  bound_gel(0, Ctx.c.sp, n, "<runtime:peek-compiler-val>", "<compiler:stack>");
  
  return &CompilerValues[n];
}

void push_compiler_val(Value val) {
  bound_lt(N_VALUES, Ctx.c.sp, "<runtime:push-compiler-val>", "<compiler:stack>");
  CompilerValues[Ctx.c.sp++] = val;
}

Value pop_compiler_val(void) {
  bound_gt(0, Ctx.c.sp, "<runtime:pop-interp-val>", "<compiler:stack>");
  return CompilerValues[--Ctx.c.sp];
}

CompFrame* peek_compiler_frame(int n) {
  if (n < 0)
    n += Ctx.c.fp;

  bound_gel(0, Ctx.c.fp, n, "<runtime:peek-compiler-frame>", "<compiler:frames>");

  return &CompilerFrames[n];
}

void push_compiler_frame(void) {
  bound_lt(N_FRAMES, Ctx.c.fp, "<runtime:push-compiler-frame>", "<compiler:frames>");

  CompilerFrames[Ctx.c.fp++] = (CompFrame) {
    .envt = Ctx.c.envt,
    .code = Ctx.c.code,
    .vals = Ctx.c.vals,
  };
}

void pop_compiler_frame(void) {
  bound_gt(0, Ctx.c.fp, "<runtime:pop-compiler-frame>", "<compiler:frames>");

  CompFrame* frame = &CompilerFrames[--Ctx.c.fp];

  Ctx.c.envt = frame->envt;
  Ctx.c.code = frame->code;
  Ctx.c.vals = frame->vals;
}

void vm_mark_compiler(void) {
  mark(Ctx.c.envt);
  mark(Ctx.c.code);
  mark(Ctx.c.vals);

  mark_compiler_frames();
}

/* Initialization. */
void vm_init_compiler(void) {
  Ctx.c = (Compiler) {
    .envt=NULL,
    .code=NULL,
    .vals=NULL,
    .fp  =0,
    .sp  =0,
  };
}
