#include <stdio.h>

#include "list.h"
#include "memory.h"
#include "prin.h"

// globals --------------------------------------------------------------------
List NilObj;

// static helpers -------------------------------------------------------------
static void initCell( List *ob, Value head, List *tail )
{
  listHead(ob)  = head;
  listTail(ob)  = tail;

  listArity(ob) = 1 + listArity(tail);
}

static void initCells( List *head, Arity nArgs, Value *args, List *last )
{
  for (Arity i=0; i<nArgs; i++)
    {
      List *tail = i == nArgs - 1 ? last : head+i+1;
      initObj( (Obj*)(head+i), VAL_LIST, true );


      if (args == NULL)
	initCell( head+i, NIL, tail );
	
      else
	initCell( head+i, args[i], tail );
    }
}

// memory methods -------------------------------------------------------------
Void traceList( Obj *ob )
{
  List *xs = (List*)ob;

  while (!objBlack(xs))
    {
      mark( listHead(xs) );
      xs = listTail(xs);
    }
}

Void  initList( Obj *object, Size count, Void *data, Flags fl )
{
  List  *list = (List*)object;
  Value *vals = data;
  List  *tail = vals && isSharedFl(fl) ? asList( vals[count-1] ) : NULL;

  initCells( list, count, vals, tail );
}

// utility methods ------------------------------------------------------------
Int  orderLists( Value xs, Value ys )
{
  if (xs == NIL)
    return -1 + (ys == NIL);

  if (ys == NIL)
    return 1;

  List *xsl = asList(xs), *ysl = asList(ys);

  Int o;

  while (xsl && ysl)
    {
      if ((o=orderValues(xsl->head, ysl->head)))
	return o;

      xsl = xsl->tail;
      ysl = ysl->tail;
    }

  return 0 - (xsl == NULL) + (ysl == NULL);
}

// IO methods -----------------------------------------------------------------
Void prinListObj( Obj *obj )
{
  printf( "(" );

  List *xsOb = asList(obj);

  while ( xsOb )
    {
      prinLisp( xsOb->head );

      
      if (!listIsProper(xsOb))
	{
	  printf( " . " );
	  prinLisp( xsOb->cdr );
	  break;
	}

      if (xsOb->tail)
	printf( " " );

      xsOb = xsOb->tail;
    }

  printf( ")" );
}

// object api -----------------------------------------------------------------
List *ListN( Value *args, Arity nArgs )
{
  return (List*)constructObj( VAL_LIST, nArgs, args, ALLOC_ALLOCATE );
}

List *Cons2( Value head, List *tail )
{
  List *out = ListN( NULL, 1 );
  initCell( out, head, tail );
  return out;
}

List *ConsN( Value *args, Arity nArgs )
{
  assert(nArgs >= 2);

  List *out  = ListN( NULL, nArgs - 1 );
  List *last = asList(args[nArgs-1]);
  initCells( out, nArgs, args, last );
  return out;
}

// initialization -------------------------------------------------------------
Void listInit( Void )
{
  initObj( (Obj*)&NilObj, VAL_LIST, false );

  NilObj.head = NIL;
  NilObj.tail = &NilObj;

  BaseSizeDispatch[VAL_LIST]   = sizeof(List);
    TraceDispatch[VAL_LIST]      = traceList;
  InitializeDispatch[VAL_LIST] = initList;
  PrinObjDispatch[VAL_LIST]    = prinListObj;
  OrderDispatch[VAL_LIST]      = orderLists;
}
