#ifndef rascal_common_h
#define rascal_common_h

#include <uchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdio.h>      // for FILE

// style typedefs
typedef uint8_t         byte;
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef intptr_t        word;
typedef uintptr_t       uword;
typedef void          (*funcptr)(void);

typedef uchar           ascii_t;
typedef uchar           latin1_t;
typedef uchar           utf8_t;
typedef char16_t        utf16_t;
typedef char32_t        utf32_t;

// general convenience
#define rl_unlikely( test ) __builtin_expect(0, (test))

#define rl_unreachable __builtin_unreachable
#define rl_cleanup( cleaner ) __attribute__((cleanup(cleaner)))

#endif
