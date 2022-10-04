#include "envt.h"
#include "atom.h"
#include "memory.h"

// var_t implementation
OBJ_NEW(var);


// envt_t implementation
typedef enum
  {
    envt_fl_toplevel=0x01,
  } envt_fl_t;

static inline ord_t cmp_atoms(atom_t *x, atom_t *y)
{
  if (x == y)
    return 0;

  if (x < y)
    return -1;

  return 1;
}

OBJ_NEW(envt);
TABLE_INIT(envt, var, ENVT);
TABLE_TRACE(envt, var);
TABLE_FREE(envt, var);
TABLE_CLEAR(envt, var);
TABLE_REHASH(envt, var);
TABLE_RESIZE(envt, var, ENVT);
TABLE_PUT(envt, var, atom_t*, name, atom_hash, cmp_atoms);
TABLE_GET(envt, var, atom_t*, name, atom_hash, cmp_atoms);

// initialization
void envt_init( void )
{
  init_envt( &Toplevel );
}
