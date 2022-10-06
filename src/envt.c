#include "envt.h"
#include "atom.h"
#include "memory.h"
#include "port.h"

// var_t implementation
OBJ_NEW( var );

void init_var( var_t *var, atom_t *name )
{
  init_obj( &var->obj, VAR, var_fl_toplevel );
  var->name = name;
  var->bind = NUL;
}

void trace_var( obj_t *obj  )
{
  var_t *var = (var_t*)obj;

  mark_obj((obj_t*)var->name);
  mark_val(var->bind);
}

void prin_var( port_t *port, val_t val )
{
  port_prinf( port, "#'%s", as_var(val)->name );
}

static inline ord_t cmp_atoms( atom_t *x, atom_t *y )
{
  if ( x == y )
    return 0;

  if ( x < y )
    return -1;

  return 1;
}

OBJ_NEW( envt );
ORDERED_TABLE_INIT( envt, var, ENVT );
TABLE_MARK( envt, var );
ORDERED_TABLE_FREE( envt, var );
TABLE_CLEAR( envt, var );
ORDERED_TABLE_RESIZE( envt, var, ENVT );
ORDERED_TABLE_REHASH( envt, var );
ORDERED_TABLE_PUT( envt, var, atom_t*, name, atom_hash, cmp_atoms );

// initialization
void envt_init( void )
{
  init_envt( &Toplevel );
}
