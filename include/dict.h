#ifndef rl_dict_h
#define rl_dict_h

#include "obj.h"

/* C types */

struct dict_t {
  objs_t  *entries;
  size_t   nkeys;
  size_t   capacity;
  void   **mapping;
};

/* globals */
extern struct type_t DictType;

/* API */
dict_t make_dict(void);

size_t dict_len(dict_t d);
int    dict_locate(dict_t d, val_t k);
bool   dict_has(dict_t d, val_t k);
bool   dict_add(dict_t d, val_t k);
bool   dict_rmv(dict_t d, val_t k);
val_t  dict_ref(dict_t d, val_t k);
val_t  dict_set(dict_t d, val_t k, val_t v);

#endif
