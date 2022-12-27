#ifndef rl_prim_h
#define rl_prim_h

#include "func.h"

/* C types */
struct prim_t {
  opcode_t label;
};

/* globals */
extern struct type_t PrimType;

/* API */
prim_t make_prim(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, opcode_t label);
val_t  prim(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, opcode_t label);

/* convenience */
#define is_prim(x) has_type(x, &PrimType)
#define as_prim(x) ((prim_t)as_obj(x))

#endif
