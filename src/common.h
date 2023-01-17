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

typedef uint8_t         uint8;
typedef int8_t          int8;
typedef uint16_t        uint16;
typedef int16_t         int16;
typedef uint32_t        uint32;
typedef int32_t         int32;
typedef int64_t         int64;
typedef uint64_t        uint64;

// C 23 typedefs
typedef void           *nullptr;

// miscellaneous utility typedefs
typedef ulong           uhash;

// character encoding typedefs
typedef char            ascii;
typedef char            latin1;
typedef char            utf8;
typedef char16_t        utf16;
typedef char32_t        utf32;

// general convenience
#define NOTHING ((void)0)
#define unlikely( test ) __builtin_expect(0, (test))
#define fallthrough __attribute__((fallthrough))
#define unreachable __builtin_unreachable
#define generic _Generic
#define attr( ... ) __attribute__((__VA_ARGS__))
#define manage(manager) __attribute__((cleanup(cleaner)))

static inline bool flagp( uint64 fls, uint64 fl ) {
  return !!(fls&fl);
}

#define rl_abort(fmt, ...)                                              \
  do {                                                                  \
    printf((fmt) __VA_OPT__(,) __VA_ARGS__);                            \
    abort();                                                            \
  } while (false)

// debug flag
#define DEBUG

#endif
