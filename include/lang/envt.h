#ifndef rl_lang_envt_h
#define rl_lang_envt_h

#include "val/envt.h"

/* Internal APIs for working with environments. */
Binding* resolve(Symbol* name, Envt* envt, bool capture);
Binding* defvar(Symbol* name, Envt* envt);
Binding* defval(Symbol* name, Envt* envt);
Binding* defun(Symbol* name, Envt* envt);
Binding* defmac(Symbol* name, Envt* envt);

#endif
