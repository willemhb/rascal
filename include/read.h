#ifndef rascal_read_h
#define rascal_read_h

#include "object.h"

// globals
extern Value QuoteSym;

// external API
// reading expressions
Value read(void);
Value read_line(void);

// initialization
void init_builtin_readers(void);

#endif
