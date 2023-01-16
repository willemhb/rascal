#ifndef rl_common_h
#define rl_common_h

#include <uchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdio.h>      // for FILE
#include <math.h>       // for nan

// style typedefs
typedef uint8_t         ubyte;
typedef size_t          usize;
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef intptr_t        word;
typedef uintptr_t       uword;
typedef void          (*funcptr)(void);

// C 23 typedefs
typedef void           *nullptr_t;

// character encoding typedefs
typedef char            ascii_t;
typedef char            latin1_t;
typedef char            utf8_t;
typedef char16_t        utf16_t;
typedef char32_t        utf32_t;

// general convenience
#define NOTHING ((void)0)
#define rl_unlikely( test ) __builtin_expect(0, (test))
#define rl_fallthrough __attribute__((fallthrough))
#define rl_unreachable __builtin_unreachable
#define rl_attr( ... ) __attribute__((__VA_ARGS__))
#define rl_cleanup( cleaner ) __attribute__((cleanup(cleaner)))
#define rl_nan nan("")

static inline bool flagp( uint64_t fls, uint64_t fl ) { return !!(fls&fl); }

#define rl_abort(fmt, ...)                                              \
  do {                                                                  \
    printf((fmt) __VA_OPT__(,) __VA_ARGS__);                            \
    abort();                                                            \
  } while (false)

// debug flag
#define DEBUG

#endif
