#ifndef apply_h
#define apply_h

#include "value.h"

/* API */
Val  apply(Val func, Val args);
Val  invoke(Val func, int n_args, Val *args);
void validate_funcall(Val func, int n_args, Val *args);

#endif
