#ifndef rascal_read_h
#define rascal_read_h

#include "common.h"
#include "vm.h"

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
