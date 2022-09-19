#ifndef rascal_list_h
#define rascal_list_h

#include "object.h"
#include "vm.h"
#include "table.h"

// C types --------------------------------------------------------------------
typedef struct
{
  UInt16               : 13;
  UInt16 isProper      :  1;
  UInt16 closedUpvalue :  1;
  UInt16 openUpvalue   :  1;
} ListFl;

struct List
{
  OBJ_HEAD(ListFl);

  union
  {
    Value car;
    Value head;
  };

  union
  {
    Value  cdr;
    List  *tail;
  };
};

// forward declarations -------------------------------------------------------
#define Cons(ca, cd)				\
  _Generic((cd),				\
	   List*:Cons2,				\
	   Arity:ConsN)((ca), (cd))

List  *Cons2(  Value head,  List *tail );
List  *ConsN( Value *args, Arity nArgs );
List  *ListN( Value *args, Arity nArgs );

// initialization -------------------------------------------------------------
Void listInit( Void );

// utility macros & statics ---------------------------------------------------
#define isList(val)            (isObjType(val), VAL_LIST)
#define asList(val)            (asObjType(List, val))

#define listOpenUpvalue(val)   (asList(val)->object.flags.openUpvalue)
#define listClosedUpvalue(val) (asList(val)->object.flags.closedUpvalue)
#define listIsProper(val)      (asList(val)->object.flags.isProper)

#define listArity(val)       (asList(val)->object.arity)
#define listCar(val)         (asList(val)->car)
#define listCdr(val)         (asList(val)->cdr)
#define listHead(val)        (asList(val)->head)
#define listTail(val)        (asList(val)->tail)

#endif
