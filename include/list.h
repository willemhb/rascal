#ifndef rascal_list_h
#define rascal_list_h

#include <stddef.h>

#include "object.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    cons_fl_continuation=16,
    cons_fl_closure     = 8,
    cons_fl_entry       = 4,
    cons_fl_cycle       = 2,
    cons_fl_proper      = 1,
  } cons_fl_t;

struct cons_t
{
  HEADER;
  value_t car, cdr;
};

// forward declarations -------------------------------------------------------
value_t cons(value_t ca, value_t cd);
value_t consn(value_t *args, size_t n_args);
value_t listn(value_t *args, size_t n_args);

// macros & statics -----------------------------------------------------------
#define as_cons(x) asa(cons_t*, x, pval)

#define car(x)    getf(cons_t*, x, car)
#define cdr(x)    getf(cons_t*, x, cdr)

static inline tag_p(list, LIST)
static inline value_p(nil, NIL)
static inline non_empty_tag_p(cons, LIST)

static inline ob_flag_p(proper, LIST, cons_fl_proper)
static inline ob_flag_p(cycle, LIST, cons_fl_cycle)
static inline ob_flag_p(entry, LIST, cons_fl_entry)
static inline ob_flag_p(closure, LIST, cons_fl_closure)
static inline ob_flag_p(continuation, LIST, cons_fl_continuation)

#endif
