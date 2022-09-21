#include "obj/cons.h"
#include "mem.h"

// globals --------------------------------------------------------------------
cons_t nil_obj;

#define NIL_LIST (&nil_obj)
#define NIL_OBJ ((obj_t*)NIL_LIST)

// forward declarations -------------------------------------------------------
obj_t *cons_construct( );

// initialization -------------------------------------------------------------
rt_table_t ConsTableObj;

static void nil_init( void )
{
  init_obj( NIL_OBJ, NIL_TYPE, MEM_STATIC );

  nil_obj.hd = NIL;
  nil_obj.tl = NIL_LIST;

  WellKnownObjects[NIL_TYPE]  = NIL_OBJ;
  WellKnownObjects[CONS_TYPE] = NIL_OBJ;
}

static void types_init( void )
{
  rt_table_t *ConsTable = &ConsTableObj;

  init_obj( (obj_t*)ConsTable, NIL_TYPE, MEM_STATIC );
}

void cons_init( void )
{
  nil_init();
  types_init();
}
