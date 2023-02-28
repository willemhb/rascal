#ifndef error_h
#define error_h

#include "value.h"
#include "htable.h"

// C types --------------------------------------------------------------------
typedef enum {
  NO_ERROR,
  READ_ERROR,
  EVAL_ERROR,
  APPLY_ERROR
} error_t;

// API ------------------------------------------------------------------------
error_t raise_error(error_t err, value_t agitant, const char* fmt, ...);
error_t catch_error(value_t* agbuf);
bool    panicking(void);

#define panic(err, ag, fmt, ...)				\
  do {								\
    raise_error(err, ag, fmt __VA_OPT__(,) __VA_ARGS__);	\
    return ag;							\
  } while (false)

#define repanic(ag)                             \
  do {                                          \
    if (panicking())                            \
      return ag;                                \
  } while (false)

#define require(test, args...)                  \
  do {                                          \
    if (!(test))                                \
      panic(args);                              \
  } while (false)

#define argco(expect, got)						\
  require((expect)==(got),						\
          APPLY_ERROR,							\
          NUL,                                  \
          "bad arity: expected %zu, got %zu",   \
          expect,                               \
          got)

#define argtype(expect, val)                    \
  require((expect)==type_of((val)),             \
          APPLY_ERROR,                          \
          NUL,                                  \
          "bad type: expected %s",              \
          type_name((expect)))

#define argval(expect, val, valstr)             \
  require((expect)==(val),                      \
          APPLY_ERROR,                          \
          NUL,                                  \
          "bad value: expected %s",             \
          (valstr))

#define argval_c(expect_c, val, valstr)         \
  require((expect_c) != (val),                  \
          APPLY_ERROR,                          \
          NUL,                                  \
          "bad value: %s",                      \
          (valstr))

#define argtest(test, val, msg, ...)            \
  require(test(val),                            \
          APPLY_ERROR,                          \
          NUL,                                  \
          msg __VA_OPT__(,)                     \
          __VA_ARGS__)

#define recover(agbuf) catch_error(agbuf)

// initialization -------------------------------------------------------------
void error_init(void);

#endif
