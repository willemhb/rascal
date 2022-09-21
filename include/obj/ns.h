#ifndef rascal_ns_h
#define rascal_ns_h

#include "obj.h"

/**
 * static environment representation.
 **/

// C types --------------------------------------------------------------------
typedef struct ns_t ns_t;

struct ns_t
{
  OBJ_HEAD;

  obj_t  *next;

  obj_t  *module;
  obj_t  *local;
  obj_t  *captured;
  obj_t  *global;
  obj_t  *macro;
};

// globals --------------------------------------------------------------------
extern obj_t *ToplevelNS;
// forward declarations -------------------------------------------------------
// toplevel dispatch ----------------------------------------------------------
void stack_mark( void );
void stack_init( void );
// convenience ----------------------------------------------------------------

#endif
