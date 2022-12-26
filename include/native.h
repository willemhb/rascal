#ifndef rl_native_h
#define rl_native_h

#include "func.h"

/* C types */
struct native_t {
  native_fn_t funptr;
};

/* globals */
extern struct type_t NativeType;

/* API */
native_t make_native(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, native_fn_t funptr);
val_t    native(char *name, int nargs, bool vargs, guard_fn_t guard, type_t type, native_fn_t funptr);

/* initialization */
void native_init(void);

/* convenience */
#define is_native(x) has_type(x, &NativeType)
#define as_native(x) ((native_t)as_obj(x))

#endif
