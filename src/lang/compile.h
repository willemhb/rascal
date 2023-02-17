#ifndef compile_h
#define compile_h

#include "object.h"

/* C types */
/* API */
UserFn *compile(Val x);
void    compile_file(char *fname);

#endif
