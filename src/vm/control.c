#include "vm/interpreter.h"
#include "vm/control.h"

#include "val/control.h"

/* External APIs */
void install_cntl(Control* cntl) {
  Value* vals_base = write_interp_vals(cntl->values, cntl->n_vals);
  ExecFrame* frames_base = write_interp_frames(cntl->frames, cntl->n_frames);

  
}

