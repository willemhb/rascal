#ifndef rascal_envt_h
#define rascal_envt_h

#include "table.h"

struct var_t
{
  obj_t obj;
  ENTRY_SLOTS(atom_t*, name, val_t, bind); 
};

struct envt_t
{
  obj_t obj;
  TABLE_SLOTS(var_t*);
};

// globals
extern envt_t Toplevel;

// forward declarations
var_t  *new_var( void );
void    init_var( var_t *var );
void    trace_var( obj_t *obj );
void    free_var( obj_t *obj );
void    prin_var( port_t *port, val_t val );

envt_t *new_envt( void );
void    init_envt( envt_t *envt );
void    trace_envt( obj_t *obj );
void    free_envt( obj_t *obj );
void    clear_envt( envt_t *envt );
void    rehash_envt( var_t **old, size_t oldc, var_t **new, size_t newc );
void    resize_envt( envt_t *envt, size_t newl );
bool    envt_put( envt_t *envt, atom_t *name, var_t **buf );
bool    envt_get( envt_t *envt, atom_t *name, var_t **buf );

// initialization
void envt_init( void );

#endif
