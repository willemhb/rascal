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
void  initReader(Vm* vm);
void  freeReader(Vm* vm);
void  resetReader(Vm* vm);
void  syncReader(Vm* vm);

Value read(void);
Value readLine(void);

#endif
