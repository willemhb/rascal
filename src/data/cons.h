#ifndef cons_h
#define cons_h

#include "base/object.h"

/* C types */
typedef enum ConsFl ConsFl;

enum ConsFl {
  PROPER_CONS=0x1,
  
};

struct Cons {
  Obj obj;

  Val car;

  union {
    Cons* tail;
    Val cdr;
  };
};

/* globals */

/* API */
bool is_cons(Val x);
bool is_proper(Val x);
Cons* as_cons(Val x);

#define CAR(x)  (as_cons(x)->car)
#define CDR(x)  (as_cons(x)->cdr)
#define TAIL(x) (as_cons(x)->tail)

#endif
