#ifndef rascal_envt_h
#define rascal_envt_h

#include "table.h"

typedef enum
  {
    var_fl_toplevel=0x0001,
  } var_fl_t;

struct var_t
{
  obj_t obj;
  ENTRY_SLOTS(atom_t*, name, val_t, bind);
};

// envt_t implementation
typedef enum
  {
    envt_fl_toplevel=0x01,
  } envt_fl_t;

struct envt_t
{
  obj_t obj;
  ORDERED_TABLE_SLOTS( var_t* );
};

// globals
extern envt_t Toplevel;

// forward declarations
var_t  *new_var( void );
void    init_var( var_t *var, atom_t *name );
void    trace_var( obj_t *obj );
void    prin_var( port_t *port, val_t val );

envt_t *new_envt( void );
void    init_envt( envt_t *envt );
void    trace_envt( obj_t *obj );
void    free_envt( obj_t *obj );
void    clear_envt( envt_t *envt );
void    rehash_envt( var_t **new, arity_t len, arity_t cap, ords_t ords );
void    resize_envt( envt_t *envt, arity_t newl );
bool    envt_put( envt_t *envt, atom_t *name, var_t **buf );
bool    envt_get( envt_t *envt, atom_t *name, var_t **buf );

// initialization
void envt_init( void );

// convenience
#define as_var(val)   ((var_t*)as_ptr(val))
#define var_name(val) (as_var(val)->name)
#define var_bind(val) (as_var(val)->bind)
#define is_var(val)   isa(val, VAR)
#define as_envt(val)  ((envt_t*)as_ptr(val))
#define is_envt(val)  isa(val, ENVT)

#endif
