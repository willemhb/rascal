#ifndef rl_lang_envt_h
#define rl_lang_envt_h

#include "val/envt.h"

/* Internal APIs for working with environments. */
Binding* resolve(Symbol* name, Envt* envt, bool capture);
Binding* defvar(Symbol* name, Envt* envt, Dict* meta);
Binding* defval(Symbol* name, Envt* envt, Dict* meta);
Binding* defun(Symbol* name, Envt* envt, Dict* meta);
Binding* defmac(Symbol* name, Envt* envt, Dict* meta);

#endif
