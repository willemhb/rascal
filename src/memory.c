#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "vm.h"

#define GC_HEAP_GROW_FACTOR 2

// trace methods --------------------------------------------------------------
Void traceValue( Value val )
{
  assert(isObj(val));
  traceObject( asObj(val) );
}

Void traceObject( Obj *object )
{
  assert(object != NULL);
  ValueType objT = typeOf(object);
  assert(TraceDispatch[objT] != NULL);

  TraceDispatch[objT]( object );
}

Void traceValues( Value *values, Arity count )
{
  for (Arity i=0; i<count; i++)
    mark(values[i]);
}

Void traceObjects( Obj **objects, Arity count )
{
  for (Arity i=0; i<count; i++)
    mark(objects[i]);
}

// finalize implementation ----------------------------------------------------
Void finalizeValue( Value arg )
{
  finalizeObject( asObj(arg) );
}

Void finalizeObject( Obj *obj )
{
  if (obj == NULL)
    return;

  ValueType oType = typeOf(obj);

  if (FinalizeDispatch[oType])
    FinalizeDispatch[oType](obj);

  if ( objAlloc(obj) )
    free( obj );
}

Void finalizeValues( Value *values, Arity count )
{
  for (Arity i=0; i<count; i++)
    finalize(values[i]);
}

Void finalizeObjects( Obj **objects, Arity count )
{
  for (Arity i=0; i<count; i++)
    finalize(objects[i]);
}

// memory management ----------------------------------------------------------
Obj  *constructObj( ValueType type, Size count, Void *data, Flags fl )
{
  Obj *obj;
  if (isAllocFl(fl))
    obj = allocate( type, count, data, fl );

  else
    {
      assert(data);
      obj = data;
    }

  if (obj != NULL)
    {
      initObj( obj, type, fl );

      if ( InitializeDispatch[type] )
	InitializeDispatch[type]( obj, count, data, fl );
    }

  return obj;
}

Void *allocateBytes( Size nBytes )
{
  vm.bytesAllocated += nBytes;

  if (vm.bytesAllocated > vm.nextGC)
    collectGarbage();

  Void *out = safeMalloc( nBytes );

  memset( out, 0, nBytes );
  return out;
}

Obj  *allocateObject( ValueType objType, Size count, Void *data, Flags fl )
{
  (Void)data;

  Obj *out;
  
  if (ConstructDispatch[objType])
    out = ConstructDispatch[objType]( count, fl );

  else if (count > 0)
    {
      Size base = BaseSizeDispatch[objType];
      out       = allocate( base * count );
    }

  else if (Immutable[objType])
    out = Singletons[objType];

  else
    {
      Size base = BaseSizeDispatch[objType];
      out       = allocateBytes(base);
    }

  return out;
}

Void *reallocateBytes( Void* pointer, Size oldSize, Size newSize )
{
  vm.bytesAllocated += newSize;
  vm.bytesAllocated -= oldSize;
  
  if (vm.bytesAllocated > vm.nextGC)
    collectGarbage();
  
  if (newSize == 0)
    {
      free(pointer);
      return NULL;
    }

  void *result = safeRealloc( pointer, newSize );

  return result;
}

Value  reallocateVal( Value val, Size newSize )
{
  if (isImmediate(val) || isObj(val))
    {
      Obj *new = reallocateObj( asObj(val), newSize );
      return tagValue( new, OBJECT );
    }

  return val;
}

Obj   *reallocateObj( Obj *object, Size newSize )
{
  if (objSingleton(object))
      return constructObj( typeOf(object), newSize, NULL, ALLOC_ALLOCATE );

  if (ResizeDispatch[typeOf(object)])
    return ResizeDispatch[typeOf(object)](object, newSize );

  // this shouldn't ever be reached
  unreachable();
}

// object and value size ------------------------------------------------------
Size valSizeOf( Value val )
{
  if (isObj(val))
    return objSizeOf( asObj(val) );

  return BaseSizeDispatch[typeOf(val)];
}

Size objSizeOf( Obj *obj )
{
  assert(obj);

  if (!objSingleton(obj) && SizeOfDispatch[objType(obj)])
    return SizeOfDispatch[objType(obj)](obj);

  return BaseSizeDispatch[objType(obj)];
}

// garbage collector phases ---------------------------------------------------
Void markObject(Obj* object) {
  if (object == NULL)
    return;

  if (object->black)
    return;

  object->black = true;

  if (objGray( object ) && TraceDispatch[objType(object)])
    arrListPush( vm.grayStack, tagValue( object, OBJECT ));

  else
    objGray( object ) = false;
}

void markValue(Value value)
{
  if (isObj(value))
    markObject(asObj(value));
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
      Value toTrace     = arrListPop( vm.grayStack );
      trace( toTrace );
      objGray( toTrace ) = false;
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

	  finalize( unreached );
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
      finalize(object);
      object = next;
    }
  
  free(vm.grayStack);
}

// initialization -------------------------------------------------------------
Void initMemory( Void )
{
  // initialize dispatch tables
  initDispatch(TraceDispatch);
  initDispatch(FinalizeDispatch);
  initDispatch(BaseSizeDispatch);
  initDispatch(SizeOfDispatch);
  initDispatch(ResizeDispatch);
  initDispatch(ConstructDispatch);
  initDispatch(InitializeDispatch);
}
