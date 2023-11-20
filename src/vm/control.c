#include "vm/interpreter.h"
#include "vm/control.h"

#include "val/control.h"

/* External APIs */
void install_cntl(Control* cntl) {
  /* save the current execution state */
  push_interp_frame();

  /* get the offset of */
  size_t bp = Ctx.i.sp;
  size_t cp = Ctx.i.cp;
  ExecFrame* frames = write_interp_frames(cntl->frames, cntl->n_frames);
  write_interp_vals(cntl->values, cntl->n_vals);

  for (size_t i=0; i<cntl->n_frames; i++) {
    frames[i].bp += bp;
    frames[i].cp  = cp;
  }
}

Control* capture_cntl(size_t cp) {
  ExecFrame* bf = peek_interp_frame(cp);
  size_t bbp = bf->bp;
  size_t nval = Ctx.i.sp - bbp;
  size_t nfrm = Ctx.i.fp - cp;
  Value* vf = peek_interp_val(-nval);

  Control* out = mk_cntl(Ctx.i.code, Ctx.i.effh, Ctx.i.ip, nfrm, nval, bf, vf);

  for (size_t i=0; i<nfrm; i++) {
    out->frames[i].bp -= bbp;
    out->frames[i].cp  = 0;
  }

  pop_interp_vals(nval);
  pop_interp_frames(nfrm);

  return out;
}

