#ifndef rl_namespc_h
#define rl_namespc_h

#include "obj.h"

/* C types */
typedef enum var_ref_t var_ref_t;

enum var_ref_t {
  var_ref_global,
  var_ref_nonlocal,
  var_ref_local
};

struct namespc_t {
  namespc_t parent;
  vec_t     locals;
  vec_t     nonlocals;
  vec_t     captured;
};

/* globals */
extern struct type_t NamespcType;

/* API */
namespc_t make_namespc(namespc_t p_e);
int       prep_namespc(namespc_t e, vec_t f, val_t b);
var_ref_t lookup_in_namespc(namespc_t e, val_t n, int *o);

/* convenience */
#define is_namespc(x) has_type(x, &NamespcType)
#define as_namespc(x) ((namespc_t)as_obj(x))

#endif
