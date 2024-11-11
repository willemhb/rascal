#ifndef rl_common_h
#define rl_common_h

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <uchar.h>

/* Utility typedefs. */
typedef uint8_t    uchar;
typedef uint16_t   ushort;
typedef uint32_t   uint;
typedef uint64_t   ulong;
typedef uint8_t    byte;
typedef uint64_t   word_t;
typedef uint64_t   hash_t;
typedef void     (*funcptr_t)(void);

// explicitly sizee types
typedef int8_t     sint8;
typedef int16_t    sint16;
typedef int32_t    sint32;
typedef int64_t    sint64;

typedef uint8_t    uint8;
typedef uint16_t   uint16;
typedef uint32_t   uint32;
typedef uint64_t   uint64;

typedef uint8_t    size8;
typedef uint16_t   size16;
typedef uint32_t   size32;
typedef size_t     size64;

typedef uint8_t    flags8;
typedef uint16_t   flags16;
typedef uint32_t   flags32;
typedef uint64_t   flags64;

typedef uint32_t   hash32;
typedef uint64_t   hash64;

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
#define MAX_STACK    UINT16_COUNT
#define MAX_FRAMES   8192
#define MAX_ERROR    512
#define MIN_ARR      8ul
#define INIT_HEAP    (1 << 19) // MAX_STACK * sizeof(Val)
#define MAX_HEAP     MAX_ARITY
#define MAX_INTERN   512


/* redefining important macros with annoying names */
#define generic _Generic
#define unreachable __builtin_unreachable

// other utilitiy macros
#define cleanup(f)   __attribute__((__cleanup__(f)))
#define noreturn     __attribute__((__noreturn__))
#define unlikely(x)  __builtin_expect((x), 0)
#define likely(x)    __builtin_expect((x), 1)

#endif
