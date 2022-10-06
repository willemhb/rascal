#ifndef rascal_pair_h
#define rascal_pair_h

#include "obj.h"
#include "val.h"

// pair types
typedef enum
  {
   cons_fl_proper=0x0001,
   cons_fl_typed =0x0002,
  } cons_fl_t;

struct cons_t
{
  obj_t   obj;

  val_t   car;
  val_t   cdr;

  arity_t len;
  type_t  eltype;
};

#define is_cons(val)     isa(val, CONS)
#define as_cons(val)     ((cons_t*)as_ptr(val))
#define cons_car(val)    (as_cons(val)->car)
#define cons_cdr(val)    (as_cons(val)->cdr)

// forward declarations
cons_t   *new_cons(void);
cons_t   *new_conses(arity_t n);
void      init_conses(cons_t *cons, val_t *args, arity_t n);
void      init_cons(cons_t *cons, val_t car, val_t cdr);
void      mark_cons(obj_t *obj);

void      prin_list(port_t *port, val_t val);


#endif
