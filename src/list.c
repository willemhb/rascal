#include <stdio.h>

#include "memory.h"
#include "value.h"
#include "table.h"
#include "list.h"

// static helpers -------------------------------------------------------------
static void initCell( List *ob, Value head, List *tail )
{
  ob->head = head;
  ob->tail = tail;

  obj_arity(ob) = 1;

  if (tail)
    obj_arity(ob) += obj_arity(tail);
}

static void initCells( List *head, Arity nArgs, Value *args, List *last )
{
  for (Arity i=0; i<nArgs; i++)
    {
      List *tail = i == nArgs - 1 ? last : head+i+1;
      initCell( head+i, args[i], tail );
    }
}

// object api -----------------------------------------------------------------
List *newList( void )
{
  return (List*)allocateObject( OBJ_LIST, 1 );
}

List *newListN( Arity nArgs )
{
  assert(nArgs > 0);
  return (List*)allocateObject( OBJ_LIST, nArgs );
}

List *Cons( Value head, List *tail )
{
  List *out = newList();
  initCell( out, head, tail );
  return out;
}

List *ListN( Value *args, Arity nArgs )
{
  if (nArgs == 0)
    return NULL;

  List *out  = newListN( nArgs );

  if (args)
    initCells( out, nArgs, args, NULL );

  return out;
}

List *ConsN( Value *args, Arity nArgs )
{
  assert(nArgs >= 2);

  List *out  = newListN( nArgs - 1);
  List *last = as_list(args[nArgs-1]);
  initCells(out, nArgs, args, last);
  return out;
}

// utilities ------------------------------------------------------------------
List *listAssoc( List *list, Value key )
{
  while (list)
    {
      if ( equalValues( list->head, key ) )
	break;

      list = list->tail;
    }

  return list;
}



// implementations ------------------------------------------------------------
Int  orderLists( Value xs, Value ys )
{
  if (xs == NIL)
    return -1 + (ys == NIL);

  if (ys == NIL)
    return 1;

  List *xsl = as_list(xs), *ysl = as_list(ys);

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

Bool equalLists( Value xs, Value ys )
{
  return !!orderLists( xs, ys );
}

Void printList( Value xs )
{
  printf( "[" );

  List *xsOb = as_list(xs);

  while ( xsOb )
    {
      printValue( xsOb->head );

      xsOb = xsOb->tail;

      if ( xsOb )
	printf( ", " );
    }

  printf( "]" );
}

Hash hashList( Value xs )
{
  if (xs == NIL)
    return hashInt(xs);

  return hashPointer(as_obj(xs));
}

// memory management ----------------------------------------------------------


// initialization -------------------------------------------------------------
Void listInit( Void )
{
  TypeDispatch[OBJ_LIST] = &ListMethods;
  TypeDispatch[VAL_NIL]  = &ListMethods;
  
  ListMethods.Print      = printList;
  ListMethods.Order      = orderLists;
  ListMethods.Equal      = equalLists;
  ListMethods.Hash       = hashList;

  ListMethods.Mark       = markList;
  ListMethods.Finalize   = NULL;
}
