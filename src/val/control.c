#include "vm/memory.h"
#include "vm/interpreter.h"

#include "val/table.h"
#include "val/symbol.h"
#include "val/func.h"
#include "val/type.h"
#include "val/control.h"

/* Object APIs and declarations for the types used to represent reified continuations. */

/* Globals */
void finalize_cntl(void* obj) {
  Control* cntl = obj;

  deallocate(NULL, cntl->frames, 0);
  deallocate(NULL, cntl->values, 0);
}

void trace_cntl(void* obj) {
  Control* cntl = obj;

  mark(cntl->code);
  mark(cntl->effh);
  mark(cntl->values, cntl->n_vals);
  mark_exec_frames(cntl->frames, cntl->n_frames);
}

INIT_OBJECT_TYPE(Control,
                 .finalizefn=finalize_cntl,
                 .tracefn=trace_cntl);

/* External API */
Control* mk_cntl(Closure* code, Chunk* effh, uint16_t* ip, size_t bp, size_t cp, size_t nf, size_t nv, ExecFrame* frames, Value* vals) {
  save(2, tag(code), tag(effh));

  Control* out = new_obj(&ControlType, 0, 0, 0);

  out->code = code;
  out->effh = effh;
  out->ip = ip;
  out->bp = bp;
  out->cp = cp;
  out->values = allocate(NULL, nf*sizeof(Value));
  out->frames = allocate(NULL, nv*sizeof(ExecFrame));

  memcpy(out->values, vals, nv*sizeof(Value));
  memcpy(out->frames, frames, nf*sizeof(ExecFrame));

  return out;
}
