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
void  initList( List *list, Value head, List *tail );
void  initListN( List *lists, Value *args, int nArgs );

List  *Cons( Value head, List *tail );
List  *ListN( Value *args, int nArgs );
List  *ConsN( Value *args, int nArgs );

void   printList( Value xs );
bool   equalLists( Value xs, Value ys );
bool   orderLists( Value xs, Value ys );
hash_t hashList( Value xs );

List  *listAssoc( List *list, Value key );

// initialization -------------------------------------------------------------
void   listInit( void );

// utility macros & statics ---------------------------------------------------
#define IS_LIST(val) (isNilObjType(val, OBJ_LIST))
#define IS_CONS(val) (isObjType(val, OBJ_LIST))

#define AS_LIST(val) ((List*)AS_OBJ(val))

#endif
