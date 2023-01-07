#ifndef rl_func_h
#define rl_func_h

#include "obj.h"

#include "def/opcodes.h"

/* C types */
typedef enum func_fl_t  func_fl_t;
typedef enum func_err_t func_err_t;
typedef struct func_head_t *func_head_t;
typedef struct func_init_t *func_init_t;

enum func_err_t {
  func_no_err,
  func_not_invocable_err,
  func_arg_underflow_err,
  func_arg_overflow_err,
  func_arg_type_err,
  func_arg_value_err,
  num_func_errs
};

struct func_head_t {
  char      *name;
  int        nargs;
  bool       vargs;
  guard_fn_t guard;
  type_t     type;  // the type this function constructs (if any)

  struct obj_head_t obj;
};

struct func_init_t {
  char      *name;
  int        nargs;
  bool       vargs;
  guard_fn_t guard;
  type_t     type;
};

/* globals */
extern struct type_t FuncType;
extern val_t FuncallErrors[num_func_errs];

/* API */
void init_func(obj_t self, type_t type, size_t n, void *ini);
void prin_func(val_t val);
bool is_type(val_t self);
func_err_t validate_func(val_t func, int argc, val_t *args);

/* convenience */
#define is_func(x)   has_type(x, &FuncType)
#define as_func(x)   ((func_t)as_obj(x))
#define func_head(x) ((func_head_t)obj_start(as_obj(x)))

#endif
