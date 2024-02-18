#ifndef rl_common_h
#define rl_common_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>

/* Utility typedefs. */
typedef uint8_t    byte_t;
typedef uint64_t   word_t;
typedef uint64_t   hash_t;
typedef void     (*funcptr_t)(void);

/* Important limits. */
#define MAX_ARITY    0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2     0x0000800000000000UL
#define ARITY_WIDTH  48
#define UINT8_COUNT  (UINT8_MAX+1)
#define UINT16_COUNT (UINT16_MAX+1)

/* status codes */
typedef enum {
  /* everything is okay */
  OKAY,

  /* may or may not indicate a problem */
  NOTFOUND,

  /* unambiguous error */
  SYSTEM_ERROR,
  RUNTIME_ERROR,
  BOUNDS_ERROR,
  READ_ERROR,
  SYNTAX_ERROR,
  COMPILE_ERROR,
  EVAL_ERROR,
} rl_status_t;

static inline bool is_error_status(rl_status_t s) {
  return s >= RUNTIME_ERROR;
}

/* redefining important macros with annoying names */
#define generic _Generic

#endif
