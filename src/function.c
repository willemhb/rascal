#include "function.h"
#include "type.h"
#include "memory.h"

// method sorting


// function implementation
OBJ_NEW(function);

void init_function( function_t *function, symbol_t *name)
{
  init_object(&function->object, FUNCTION, 0 );

  function->name      = name;
  function->type_info = NULL;
  function->methods   = NULL;
  function->cache     = NULL;
}

// method implementation
