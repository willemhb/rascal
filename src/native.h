#ifndef native_h
#define native_h

#include "value.h"

// API ------------------------------------------------------------------------
void define_native(char* name, native_t native);

// initialization -------------------------------------------------------------
void native_init(void);

#endif
