#ifndef rl_common_h
#define rl_common_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>

/* Utility typedefs. */
typedef uint8_t    byte_t;
typedef uint64_t   word_t;
typedef uint64_t   hash_t;
typedef void     (*funcptr_t)(void);

#if __GNUC__ < 13

typedef void* nullptr_t;
#define nullptr NULL;

#endif

/* Important limits. */
#define MAX_ARITY    0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2     0x0000800000000000UL
#define ARITY_WIDTH  48
#define UINT8_COUNT  (UINT8_MAX+1)
#define UINT16_COUNT (UINT16_MAX+1)

/* status codes */
typedef enum {
  /* default return value */
  OKAY,

  /* table lookup status codes */
  NOTFOUND,
  ADDED,
  UPDATED,
  REMOVED,

  /* reader status codes */
  READY,
  EXPRESSION,
  END_OF_INPUT,

  /* error codes */
  /* error codes that may or may not imply a user mistake */
  SYSTEM_ERROR,     // error originating from OS, eg file not found. Usually fatal
  RUNTIME_ERROR,    // error originating from runtime, eg stack overflow

  /* error codes that always imply a user mistake */
  READ_ERROR,       // error originating from reader, eg unclosed '('
  COMPILE_ERROR,    // error originating from compiler, eg a malformed (def ...)
  EVAL_ERROR,       // error originating from interpreter, eg a failed type check
  USER_ERROR,       // error raised by the user
} rl_status_t;

static inline bool is_error_status(rl_status_t s) {
  return s >= SYSTEM_ERROR;
}

/* redefining important macros with annoying names */
#define generic _Generic
#define cleanup(f) __attribute__((cleanup(f)))

#endif
