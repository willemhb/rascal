#ifndef rascal_cell_h
#define rascal_cell_h

#include "rl/value.h"
#include "obj/type.h"

typedef struct cons_t  cons_t;
typedef struct icons_t icons_t;

struct cons_t
{
  OBJECT
  size_t  length;
  value_t head;
  cons_t *tail;
};

struct icons_t
{
  OBJECT
  size_t  length;
  value_t head;
  value_t tail;
};

// globals
extern type_t ConsType, IConsType, NulType;
extern type_t ListType, CellType;

// forward declarations
void rl_init_obj_cell( void );
void rl_mark_obj_cell( void );
void rl_unmark_obj_cell( void );
void rl_finalize_obj_cell( void );

// convenience
#define is_cons( x )    is_type( x, &ConsType )
#define is_icons( x )   is_type( x, &IConsType )

#define as_cons( x )    ( (cons_t*)as_obj( x ) )
#define as_icons( x )   ( (icons_t*)as_obj( x ) )
#define rl_head( x )    ( as_cons( x )->head )
#define rl_tail( x )    ( as_cons( x )->tail )
#define rl_clength( x ) ( as_cons( x )->length )

#endif
