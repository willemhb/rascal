#include <stdlib.h>

#include "object.h"
#include "memory.h"
#include "vm.h"


Void initObj( Obj *object, ValueType type, Flags fl )
{
  object->type  =  type;
  object->flags =  0;
  object->arity =  0;

  if (isAllocFl(fl))
    {
      object->next  = vm.objects;
      vm.objects    = object;
      object->alloc =  true;
      object->gray  =  true;
      object->black =  false;
    }

  else
    {
      object->next  = NULL;
      object->alloc = false;
      object->gray  = true;
      object->black = true;
    }
}
