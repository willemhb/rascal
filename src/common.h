#ifndef common_h
#define common_h

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

typedef uint8_t ubyte;
typedef uint32_t uint;
typedef uintptr_t uword;
typedef uintptr_t uhash;
typedef size_t usize;
typedef void (*funcptr)(void);
typedef uint32_t flags;

typedef uint8_t uint8;
typedef int8_t   sint8;
typedef uint16_t uint16;
typedef int16_t  sint16;
typedef uint32_t uint32;
typedef int32_t  sint32;
typedef uint64_t uint64;
typedef int64_t sint64;

#define RASCAL_DEBUG

// utilities ------------------------------------------------------------------
#define FAIL(status, msg, ...)                              \
  do {                                                      \
    fprintf(stderr, msg".\n" __VA_OPT__(,) __VA_ARGS__);    \
    exit(status);                                           \
  } while (false)

#define TYPE(T) T##_t

#define generic _Generic
#define attrs   __attribute__

#define generic2(method, x, ...)                                        \
  generic((x),                                                          \
          value_t:val_##method,                                         \
          default:obj_##method)(x __VA_OPT__(,) __VA_ARGS__)

#define popcnt(x)                               \
  generic((x),                                  \
          uint16_t: __builtin_popcount,         \
          uint32_t: __builtin_popcount,         \
          uint64_t: __builtin_popcountl)(x)

#define ctz(x)                      \
  generic((x),                      \
          uint32_t: __builtin_ctz,  \
	  uint64_t: __builtin_ctzl)(x)

#endif
