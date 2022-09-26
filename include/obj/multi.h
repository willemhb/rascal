#ifndef rascal_multi_h
#define rascal_multi_h

#include "obj.h"
#include "obj/envt.h"

typedef struct cons_t  cons_t;
typedef struct code_t  code_t;
typedef struct func_t  func_t;
typedef struct multi_t multi_t;

/**
 *
 * multiple dispatch algorithm
 * 
 **/

struct multi_t
{
  OBJ_HEAD;
  objs_t table;

  arity_t position;       // the argument to examine next
  type_t  type;           // the type of the last argument
  func_t *method;         // method to try on success
};

// forward declarations
bool method_get(multi_t *multi, val_t *args, arity_t n, obj_t **buf);

#endif
