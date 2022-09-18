#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "compiler.h"
#include "list.h"
#include "function.h"
#include "vm.h"

#define GC_HEAP_GROW_FACTOR 2

// C types --------------------------------------------------------------------
typedef Void (*Trace)(Obj*);
typedef Void (*Finalize)(Obj *);

// forward declarations -------------------------------------------------------
Void traceList(Obj *obj);
Void traceFunction(Obj *obj);
Void traceVector(Obj *obj);
Void traceMap(Obj *obj);
Void traceClosure(Obj *obj);

Void traceUpValue(Obj *obj);

Void traceArrayList(Obj *obj);
Void traceByteCode(Obj *obj);
Void traceTable(Obj *obj);

Void finalizeAtom(Obj *obj);
Void finalizeFunction(Obj *obj);
Void finalizeBinary(Obj *obj);
Void finalizeVector(Obj *obj);
Void finalizeMap(Obj *obj);
Void finalizeClosure(Obj *obj);

Void finalizeArrayList(Obj *obj);
Void finalizeByteCode(Obj *obj);
Void finalizeTable(Obj *obj);

Trace TraceDispatch[NUM_TYPES] =
  {
    [VAL_REAL]      = NULL,           [VAL_BOOL]      = NULL,
    [VAL_CHAR]      = NULL,

    [VAL_ATOM]      = NULL,           [VAL_LIST]      = traceList,
    [VAL_FUNCTION]  = traceFunction,  [VAL_BINARY]    = NULL,
    [VAL_VECTOR]    = traceVector,    [VAL_MAP]       = traceMap,
    [VAL_CLOSURE]   = traceClosure,

    [VAL_UPVALUE]   = traceUpValue,   [VAL_NATIVE]    = NULL,
    [VAL_ARRAYLIST] = traceArrayList, [VAL_BYTECODE]  = traceByteCode,
    [VAL_TABLE]     = traceTable,     [VAL_ENTRY]     = NULL,
  };

Finalize FinalizeDispatch[NUM_TYPES] =
  {
    [VAL_REAL]      = NULL,              [VAL_BOOL]      = NULL,
    [VAL_CHAR]      = NULL,

    [VAL_ATOM]      = finalizeAtom,      [VAL_LIST]      = NULL,
    [VAL_FUNCTION]  = finalizeFunction,  [VAL_BINARY]    = finalizeBinary,
    [VAL_VECTOR]    = finalizeVector,    [VAL_MAP]       = finalizeMap,
    [VAL_CLOSURE]   = finalizeClosure,

    [VAL_UPVALUE]   = NULL,              [VAL_NATIVE]    = NULL,
    [VAL_ARRAYLIST] = finalizeArrayList, [VAL_BYTECODE]  = finalizeByteCode,
    [VAL_TABLE]     = traceTable,        [VAL_ENTRY]     = NULL,
  };

// internal methods -----------------------------------------------------------
Void traceList(Obj *obj)
{
  List *list = (List*)obj;
  
  while (list && !objBlack(list))
    {
      markValue(list->head);
      objGray(list) = false;
      list = list->tail;
    }
}

Void traceUpValue(Obj *obj)
{
  UpValue *upvalue = asUpValue(obj);

  while (upvalue && !objBlack(upvalue))
    {
      Value val = deRefUpValue(upvalue);
      markValue(val);
      objGray(upvalue) = false;
      upvalue = upValueNext(upvalue);
    }
}

Void markFunction(Obj *obj)
{
  
}


void markArrayList(Obj *obj)
{
  ArrayList *array = asArrayList(obj);
  
  for (Arity i = 0; i < array->length; i++)
    markValue(array->values[i]);
}


// memory management ----------------------------------------------------------
Void *allocate( Size nBytes )
{
  vm.bytesAllocated += nBytes;

  if ()
    
}

Void* reallocate(Void* pointer, Size oldSize, Size newSize)
{
  vm.bytesAllocated += newSize - oldSize;
  if (newSize > oldSize)
    {
#ifdef DEBUG_STRESS_GC
      collectGarbage();
#endif
      
      if (vm.bytesAllocated > vm.nextGC)
	{
	  collectGarbage();
	}
    }
  
  if (newSize == 0)
    {
      free(pointer);
      return NULL;
    }

  void *result;

  if (cache)
    {
      result = safeMalloc( newSize );
      
    }

  else
    result = safeRealloc( pointer, newSize );
  
  return result;
}

void markObject(Obj* object) {
  if (object == NULL)
    return;

  if (object->black)
    return;

  object->black = true;

  if (objGray( object ) && TraceDispatch[objType(object)])
      pushArrayList( vm.grayStack, tagObj( object ) );

  else
    objGray( object ) = false;
}

void markValue(Value value)
{
  if (isObj(value))
    markObject(asObj(value));
}


static void freeObject(Obj* object)
{
  
}

static Void markRoots( Void )
{
  markObject( (Obj*)vm.stack );
  markObject( (Obj*)vm.closure );
  markObject( (Obj*)vm.globals );
  markObject( (Obj*)vm.symbols );
  markObject( (Obj*)vm.openUpvalues );
}

static Void traceReferences( Void )
{
  while (vm.grayStack->length > 0)
    {
      Value toTrace = popArrayList( vm.grayStack );
      Obj  *object  = asObj( toTrace );
      TraceDispatch[objType(object)]( object );
    }
}

static Void sweep( Void )
{
  Obj* previous = NULL;
  Obj* object = vm.objects;
  
  while (object)
    {
      if (objBlack(object))
	{
	  objBlack(object) = false;
	  objGray(object)  = true;
	  previous         = object;
	  object           = object->next;
	}

      else
	{
	  Obj* unreached = object;
	  object         = objNext( object );
	  
	  if (previous)
	      previous->next = object;

	  else
	      vm.objects = object;

	  freeObject(unreached);
	}
    }
}

Void collectGarbage( Void )
{
  markRoots();
  traceReferences();
  sweep();
  vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;
}

Void freeObjects( Void )
{
  Obj* object = vm.objects;
  while (object != NULL)
    {
      Obj* next = object->next;
      freeObject(object);
      object = next;
    }
  
  free(vm.grayStack);
}
