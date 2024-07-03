#ifndef rl_common_h
#define rl_common_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>

/* Utility typedefs. */
typedef uint8_t    uchar;
typedef uint16_t   ushort;
typedef uint32_t   uint;
typedef uint64_t   ulong;
typedef uint8_t    byte;
typedef uint32_t   flags_t;
typedef uint64_t   word_t;
typedef uint64_t   hash_t;
typedef void     (*funcptr_t)(void);

#if __GNUC__ < 13

typedef void* nullptr_t;
#define nullptr NULL;

#endif

/* status codes and basic error handling. */
typedef enum {
  /* No error */
  OKAY,

  /* error codes */
  /* error codes that may or may not imply a user mistake */
  SYS_ERR,     // error originating from OS, eg file not found. Usually fatal
  RUN_ERR,    // error originating from runtime, eg stack overflow

  /* error codes that always imply a user mistake */
  READ_ERR,       // error originating from reader, eg unclosed '('
  COMP_ERR,       // error originating from compiler, eg 
  STX_ERR,         // error originating from compiler specifically to do with
  GENFN_ERR,      // error originating from missing method signature
  EVAL_ERR,       // error originating from interpreter, eg a failed type check
  USER_ERR,       // error raised by the user
} Error;

#define NUM_ERRS (USER_ERR+1)

/* Important limits. */
#define MAX_ARITY    0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2     0x0000800000000000UL
#define ARITY_WIDTH  48
#define UINT8_COUNT  (UINT8_MAX+1)
#define UINT16_COUNT (UINT16_MAX+1)
#define MAX_STACK    UINT16_COUNT
#define MIN_ARR      8
#define INIT_HEAP    (1 << 19) // MAX_STACK * sizeof(Val)
#define MAX_HEAP     MAX_ARITY
#define FN_FRAMEC    3

/* redefining important macros with annoying names */
#define generic _Generic
#define unreachable __builtin_unreachable

// other utilitiy macros
#define cleanup(f)   __attribute__((cleanup(f)))
#define unlikely(x)  __builtin_expect((x), 0)
#define likely(x)    __builtin_expect((x), 1)

#endif
