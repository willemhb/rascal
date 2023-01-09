#ifndef rl_method_h
#define rl_method_h

#include "obj.h"

/* C types */
typedef struct method_t       *method_t;
typedef struct method_table_t *method_table_t;
typedef enum   method_err_t    method_err_t;

struct method_t {
  int        nargs;
  bool       vargs;
  guard_fn_t guard_fn;
  val_t      handler;
};

struct method_table_t {
  method_t variadic; // at present only one variadic method is supported
  objs_t  *methods;  // fixed arity methods stored here, indexed by their arity
};

//

#endif
