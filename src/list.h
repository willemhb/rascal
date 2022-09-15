#ifndef rascal_list_h
#define rascal_list_h

#include "object.h"

// C types --------------------------------------------------------------------
struct List
{
  OBJ_HEAD;
  Value  head;
  List  *tail;
};

// forward declarations -------------------------------------------------------
List *Cons( Value head, List *tail );
List *newList( Value *args, int nArgs );

void  printList( List *xs );
bool  listsEqual( List *xs, List *ys );

#endif
