#ifndef rl_vm_control_h
#define rl_vm_control_h

#include "vm/context.h"

/* Effect handling API */

/* External APIs */
void     install_cntl(Control* cntl);
Control* capture_cntl(size_t cp);
Control* extend_cntl(Control* cntl, size_t cp);

#endif
