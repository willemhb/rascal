#ifndef rascal_vm_h
#define rascal_vm_h

#include "common.h"
#include "value.h"
#include "object.h"

// global interpreter state
typedef struct {
  // heap state
  Obj*         objects;
  size_t       heapUsed;
  size_t       heapCapacity;
  ObjectsArray grays;

  // interpreter state

  // 
} Vm;

extern Vm vm;

// forward declarations
void initVm(Vm* vm);

#endif
