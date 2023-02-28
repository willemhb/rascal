#ifndef common_h
#define common_h

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

typedef uint8_t    ubyte;
typedef uintptr_t  uword;
typedef uintptr_t  uhash;
typedef size_t     usize;
typedef void     (*funcptr)(void);
typedef uint32_t   flags;

#define RASCAL_DEBUG

// utilities ------------------------------------------------------------------
#define FAIL(status, msg, ...)					\
  do {								\
    fprintf(stderr, msg".\n" __VA_OPT__(,) __VA_ARGS__);	\
    exit(status);						\
  } while (false)

#define TYPE(T) T##_t

#define generic _Generic

#endif
