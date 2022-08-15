#ifndef rascal_list_h
#define rascal_list_h

#include "common.h"

// C types --------------------------------------------------------------------
struct pair_t {
  value_t car;
  value_t cdr;
};

// utilities ------------------------------------------------------------------
bool_t is_nil(value_t x);
bool_t is_cell(value_t x);
bool_t is_cons(value_t x);
bool_t is_pair(value_t x);
bool_t is_list(value_t x);

#define as_cons(x) as_type(cons_t*, cons_noop, opval, x)

#define car(x)     getf(cons, x, car)
#define cdr(x)     getf(cons, x, cdr)

// implementation -------------------------------------------------------------
size_t  list_print(value_t x, port_t *ios);
sint_t  list_order(value_t x, value_t y);
hash_t  list_hash(value_t x);
value_t list_relocate(value_t x);

// native functions -----------------------------------------------------------
value_t native_cons(value_t *args, size_t n_args);
value_t native_list(value_t *args, size_t n_args);
value_t native_car(value_t  *args, size_t n_args);
value_t native_cdr(value_t  *args, size_t n_args);

// initialization -------------------------------------------------------------
void list_init(void);

#endif
