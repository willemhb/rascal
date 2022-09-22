#include "obj/cons.h"
#include "mem.h"

// globals --------------------------------------------------------------------
cons_t nil_obj;

#define NIL_LIST (&nil_obj)
#define NIL_OBJ ((obj_t*)NIL_LIST)

// forward declarations -------------------------------------------------------
obj_t *construct_cons(val_type_t type, arity_t arity, void *ini, flags_t fl);
void   init_cons(obj_t *obj, arity_t arity, void *ini, flags_t fl );


// initialization -------------------------------------------------------------
rt_table_t ConsTableObj;

static void nil_init( void )
{
  init_obj( NIL_OBJ, nil_type, STATIC_OBJ );

  nil_obj.hd = NIL;
  nil_obj.tl = NIL_LIST;

  WellKnownObjects[nil_type]  = NIL_OBJ;
  WellKnownObjects[nil_type]  = NIL_OBJ;
}

static void types_init( void )
{
  rt_table_t *ConsTable = &ConsTableObj;

  init_obj( (obj_t*)ConsTable, rt_table_type, STATIC_OBJ );

  
}

void cons_init( void )
{
  nil_init();
  types_init();
}
