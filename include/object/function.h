#ifndef rascal_function_h
#define rascal_function_h

#include "core.h"

struct function_t {
  object_t base, *envt;

  union {
    object_t *formals; // interpreted
    object_t *values;  // compiled (not yet implemented)
  };

  union {
    object_t *body; // interpreted
    object_t *code; // compiled (not yet implemented)
  };
};

#define asfun(x)      ((function_t*)ptr(x))
#define funenvt(x)    (asfun(x)->envt)
#define funformals(x) (asfun(x)->formals)
#define funbody(x)    (asfun(x)->body)

#endif
