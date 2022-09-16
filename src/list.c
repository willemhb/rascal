#include <stdio.h>

#include "memory.h"
#include "list.h"

// object api -----------------------------------------------------------------
List *newList( void )
{
  return safeMalloc( __func__, sizeof( List ) );
}

List *newListN( int nArgs )
{
  return safeCalloc( __func__, nArgs, sizeof( List ) );
}



List *Cons( Value head, List *tail )
{
  List *out = newList();
  initList( out, head, tail );
  return out;
}

List *ListN( Value *args, int nArgs )
{
  if (nArgs == 0)
    return NULL;

  
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
bool equalLists( Value xs, Value ys )
{
  if (xs == NIL)
    return ys == NIL;

  if (ys == NIL)
    return false;

  
}

void printList( Value xs )
{
  printf( "[" );

  List *xsOb = AS_LIST(xs);

  while ( xsOb )
    {
      printValue( xsOb->head );

      xsOb = xsOb->tail;

      if ( xsOb )
	printf( ", " );
    }

  printf( "]" );
}
