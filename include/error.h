#ifndef rl_error_h
#define rl_error_h

#include "rascal.h"

#include "vm.h"

#define READERPANIC -2

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
