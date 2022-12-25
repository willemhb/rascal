#include <assert.h>

#include "list.h"

#include "type.h"

#include "prin.h"

/* C types */
/* globals */ 
/* nul dummy object */
obj_head_t nul_head = { nul_obj, true, false, 0, 0 };
struct cons_t nul_data = { NUL, NUL };

/* API */
/* external */
cons_t make_cons(val_t car, val_t cdr) {
  val_t buf[2] = { car, cdr };

  return (cons_t)make_obj(cons_obj, 2, buf);
}

cons_t make_conses(size_t n, val_t *args) {
  assert(n >= 2);

  if (n == 2)
    return make_cons(args[0], args[1]);
  
  cons_t out = make_cons(args[n-2], args[n-1]);

  for (size_t i=n-2; i>0; i--)
    out = make_cons(args[i-1], tag_val(out, OBJECT));

  return out;
}

cons_t make_list(size_t n, val_t *args) {
  if (n == 0)
    return NULL;

  else if (n == 1)
    return make_cons(args[0], NUL);

  else
    return make_conses(n, args);
}

val_t cons(val_t car, val_t cdr) {
  cons_t new_cons = make_cons(car, cdr);

  return tag_val(new_cons, OBJECT);
}

/* internal */
void init_cons(obj_t self, obj_type_t type, size_t n, void *ini) {
  (void)type;
  (void)n;

  ((val_t*)self)[0] = ((val_t*)ini)[0];
  ((val_t*)self)[1] = ((val_t*)ini)[1];
}


void prin_list(val_t x) {
  printf("(");

  while (is_cons(x)) {
    prin(cons_car(x));
    x = cons_cdr(x);
    
    if (is_cons(x))
      printf(" ");

    else if (!is_nul(x)) {
      printf(" . ");
      prin(x);
    }
  }

  printf(")");
}


/* generics */
#include "tpl/impl/generic.h"

ISA_METHOD(cons, val, rl_type, 1, cons_type);
ISA_METHOD(cons, obj, rl_type, 1, cons_type);
ISA_NON0(cons, cons);

ASA_METHOD(cons, val, is_cons, as_obj);
ASA_METHOD(cons, obj, is_cons, NOOP_CNVT);
ASA_METHOD(cons, cons, NON0_GUARD, NOOP_CNVT);

/* initialization */
void cons_init(void) {
  /* initialize cons type */
  Type[cons_type] = (struct dtype_t) {
    .name="cons",
    .prin=prin_list,

    .init=init_cons,

    .head_size=sizeof(struct obj_head_t),
    .base_offset=0,
    .body_size=sizeof(struct cons_t)
  };

  /* initialize nul type */
  Type[nul_type] = (struct dtype_t) {
    .name="nul",
    .prin=prin_list
  };
}
