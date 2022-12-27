#include <stdarg.h>
#include <assert.h>

#include "list.h"

#include "type.h"

#include "prin.h"

/* C types */
/* globals */
/* types */
void prin_list(val_t val);
bool isa_list(type_t self, val_t val);
void init_cons(obj_t self, type_t type, size_t n, void *ini);

struct type_t NulType = {
  .name="nul",
  .prin=prin_list
};

struct type_t ListType = {
  .name="list",
  .isa=isa_list
};

struct type_t ConsType = {
  .name="cons",
  .prin=prin_list,
  .init=init_cons,
  .head_size=sizeof(struct obj_head_t),
  .body_size=sizeof(struct cons_t)
};

/* nul object */
struct nul_obj_t NulObj = {
  .head={
    &NulType,
    sizeof(struct nul_obj_t),
    0,
    false
  },
  .body={
    NUL,
    NUL
  }
};

/* API */
/* external */
cons_t make_cons(val_t car, val_t cdr) {
  val_t buf[2] = { car, cdr };

  return (cons_t)make_obj(&ConsType, 2, buf);
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

val_t list(size_t n, ...) {
  val_t buffer[n+1];

  va_list va; va_start(va, n);

  for (size_t i=0; i<n; i++)
    buffer[i] = va_arg(va, val_t);

  va_end(va);
  buffer[n] = NUL;

  cons_t out = make_list(n+1, buffer);

  return tag_val(out, OBJECT);
}

size_t list_len(val_t xs) {
  size_t out=0;

  while (is_cons(xs)) {
    out++;
    xs = as_cons(xs)->cdr;
  }

  return out;
}

/* internal */
void init_cons(obj_t self, type_t type, size_t n, void *ini) {
  (void)type;
  (void)n;

  ((val_t*)self)[0] = ((val_t*)ini)[0];
  ((val_t*)self)[1] = ((val_t*)ini)[1];
}


void prin_list(val_t x) {
  printf("(");

  while (is_cons(x)) {
    prin(as_cons(x)->car);
    x = as_cons(x)->cdr;
    
    if (is_cons(x))
      printf(" ");

    else if (!is_nul(x)) {
      printf(" . ");
      prin(x);
    }
  }

  printf(")");
}

bool isa_list(type_t self, val_t val) {
  (void)self;

  return val == NUL || val_type_of(val) == &ConsType;
}

/* native functions */
#include "func.h"
#include "sym.h"
#include "native.h"
#include "tpl/impl/funcall.h"

func_err_t cons_method_guard(size_t nargs, val_t *args) {
  (void)nargs;
  TYPE_GUARD(cons, args, 0);
  return func_no_err;
}

func_err_t list_method_guard(size_t nargs, val_t *args) {
  (void)nargs;
  TYPE_GUARD(list, args, 0);
  return func_no_err;
}

val_t native_cons(size_t nargs, val_t *args) {
  (void)nargs;
  
  return cons(args[0], args[1]);
}

val_t native_car(size_t nargs, val_t *args) {
  (void)nargs;

  return as_cons(args[0])->car;
}

val_t native_cdr(size_t nargs, val_t *args) {
  (void)nargs;

  return as_cons(args[0])->cdr;
}

void cons_init(void) {
  val_t cons_native = native("cons", 2, false, NULL, &ConsType, native_cons);
  val_t car_native  = native("car", 1, false, cons_method_guard, NULL, native_car);
  val_t cdr_native  = native("cdr", 1, false, cons_method_guard, NULL, native_cdr);

  define("cons", cons_native);
  define("car", car_native);
  define("cdr", cdr_native);
}
