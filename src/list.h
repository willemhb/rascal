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

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
List *newList( void );
List *newListN( int nArgs );
Void  initList( List *list, Value head, List *tail );
Void  initListN( List *lists, Value *args, int nArgs );

List  *Cons( Value head, List *tail );
List  *ListN( Value *args, int nArgs );
List  *ConsN( Value *args, int nArgs );

Void   printList( Value xs );
Bool   equalLists( Value xs, Value ys );
Int    orderLists( Value xs, Value ys );
Hash   hashList( Value xs );

List  *listAssoc( List *list, Value key );

// utility macros & statics ---------------------------------------------------
#define is_list(val) (isNilObjType(val, OBJ_LIST))
#define is_cons(val) (isObjType(val, OBJ_LIST))

#define as_list(val) ((List*)AS_OBJ(val))

#endif
