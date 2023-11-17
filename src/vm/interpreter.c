#include "vm/memory.h"
#include "vm/error.h"
#include "vm/interpreter.h"

/* Globals */
#define N_VALUES 65536
#define N_FRAMES 16384

/* Value stack and call stack. */
Value     InterpreterValues[N_VALUES];
ExecFrame InterpreterFrames[N_FRAMES];

/* Internal API */
static void mark_interpreter_frame(ExecFrame* f) {
  mark(f->code);
  mark(f->effh);
}

static void mark_interpreter_frames(void) {
  for (size_t i=0; i<Ctx.i.fp; i++)
    mark_interpreter_frame(&InterpreterFrames[i]);
}

/* External API */
Value* peek_interp_val(int n) {
  if (n < 0)
    n += Ctx.i.sp;
  
  bound_gel(0, Ctx.i.sp, n, "<runtime:peek-interp-val>", "<interpreter:stack>");
  
  return &InterpreterValues[n];
}

void push_interp_val(Value val) {
  bound_lt(N_VALUES, Ctx.i.sp, "<runtime:push-interp-val>", "<interpreter:stack>");
  InterpreterValues[Ctx.i.sp++] = val;
}

Value pop_interp_val(void) {
  bound_gt(0, Ctx.i.sp, "<runtime:pop-interp-val>", "<interpreter:stack>");
  return InterpreterValues[--Ctx.i.sp];
}

ExecFrame* peek_interp_frame(int n) {
  if (n < 0)
    n += Ctx.i.fp;

  bound_gel(0, Ctx.i.fp, n, "<runtime:peek-interp-frame>", "<interpreter:frames>");

  return &InterpreterFrames[n];
}

void push_interp_frame(void) {
  bound_lt(N_FRAMES, Ctx.i.fp, "<runtime:push-interp-frame>", "<interpreter:frames>");

  InterpreterFrames[Ctx.i.fp++] = (ExecFrame) {
    .code = Ctx.i.code,
    .effh = Ctx.i.effh,
    .ip   = Ctx.i.ip,
    .bp   = Ctx.i.bp,
    .cp   = Ctx.i.cp
  };
}

void pop_interp_frame(void) {
  bound_gt(0, Ctx.i.fp, "<runtime:pop-interp-frame>", "<interpreter:frames>");

  ExecFrame* frame = &InterpreterFrames[--Ctx.i.fp];

  Ctx.i.code = frame->code;
  Ctx.i.effh = frame->effh;
  Ctx.i.ip   = frame->ip;
  Ctx.i.bp   = frame->bp;
  Ctx.i.cp   = frame->cp;
}

void vm_mark_interpreter(void) {
  mark(Ctx.i.code);
  mark(Ctx.i.effh);

  mark_interpreter_frames();
}

/* Initialization. */
void vm_init_interpreter(void) {
  Ctx.i = (Interpreter) {
    .code=NULL,
    .effh=NULL,
    .ip  =NULL,
    .bp  =0,
    .cp  =0,
    .sp  =0,
    .fp  =0,
  };
}
