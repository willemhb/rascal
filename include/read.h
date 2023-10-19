#ifndef rascal_read_h
#define rascal_read_h

#include "object.h"

/* reader, io types */
// C types
struct Stream {
  Obj     obj;
  FILE*   ios;
};

// globals
extern Value QuoteSym;

// external API
Value read(void);
Value read_line(void);

#endif
