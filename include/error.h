#ifndef rl_error_h
#define rl_error_h

#include "rascal.h"

#include "vm.h"

#define READERPANIC -2

/* C types */
typedef enum eval_err_t eval_err_t;
typedef enum read_err_t read_err_t;
typedef enum comp_err_t comp_err_t;
typedef union rl_err_t rl_err_t;

enum read_err_t {
  no_read_err,
  eos_err,
  no_reader_method_err,
};

enum comp_err_t {
  no_comp_err,
  malfored_syntax_err=no_reader_method_err+1,
  illegal_syntax_err,
  ambiguous_method_err
};

enum eval_err_t {
  no_eval_err,
  unbound_symbol_err=ambiguous_method_err+1,
  overflow_err,
  underflow_err,
  not_invocable_err,
  arg_value_err,
  arg_type_err,
  no_method_err
};

union rl_err_t {
  read_err_t read_err;
  comp_err_t comp_err;
  eval_err_t eval_err;
};

/* API */
void panic_mode(const char *fmt, ...);
bool panicking(void);
bool recover(void);

#define repanic(...)                            \
  do {                                          \
    if (panicking())                            \
      return __VA_ARGS__;                       \
  } while (false)

#define panic(out, fmt, ...)                                \
  do {                                                      \
    if (!panicking())                                       \
      panic_mode(fmt __VA_OPT__(,) __VA_ARGS__);            \
    return out;                                             \
  } while (false)

#endif
