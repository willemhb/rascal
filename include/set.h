#ifndef rl_set_h
#define rl_set_h

#include "obj.h"

/* C types */
struct set_t {
  vals_t  *entries;
  size_t   nkeys;
  size_t   capacity;
  void   **mapping;
};

/* globals */
extern struct type_t SetType;

/* API */
set_t  make_set(void);

size_t set_len(set_t s);
int    set_locate(set_t s, val_t v);
bool   set_has(set_t s, val_t v);
bool   set_add(set_t s, val_t v);
bool   set_rmv(set_t s, val_t v);

/* convenience */
#define is_set(x) has_type(x, &SetType)
#define as_set(x) ((set_t)as_obj(x))

#endif
