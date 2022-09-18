#ifndef rascal_list_h
#define rascal_list_h

#include "object.h"
#include "vm.h"
#include "table.h"

// C types --------------------------------------------------------------------
typedef struct
{
  UInt8 isOpen;
  UInt8 isClosed;
} UpValueFl;

struct List
{
  OBJ_HEAD(UInt16);
  Value  head;
  List  *tail;
};

struct UpValue
{
  OBJ_HEAD(UpValueFl);
  Value    value;
  UpValue *next;
};

// forward declarations -------------------------------------------------------
List  *Cons(  Value head,  List *tail  );
List  *ListN( Value *args, Arity nArgs );
List  *ConsN( Value *args, Arity nArgs );
List  *listAssoc( List *list, Value key );

// initialization -------------------------------------------------------------
Void listInit( Void );

// utility macros & statics ---------------------------------------------------
#define isUpValue(val)       (isObjType(val), VAL_UPVALUE)
#define asUpValue(val)       (asObjType(UpValue, val))
 
#define listHead(val)        (asList(val)->head)
#define listTail(val)        (asList(val)->tail)

#define upValueValue(val)    (asUpValue(val)->value)
#define upValueNext(val)     (asUpValue(val)->next)
#define upValueisOpen(val)   (asUpValue(val)->object.flags.isOpen)
#define upValueIsClosed(val) (asUpValue(val)->object.flags.isClosed)

static inline Value deRefUpValue(UpValue *upval)
{
  if (upValueIsClosed(upval))
    return upValueValue(upval);

  return vm.stack->values[asArity(upValueValue(upval))];
}

#endif
