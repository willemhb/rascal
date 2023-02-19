#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uchar.h>

/* style typedefs */
typedef signed char    schar;
typedef signed short   sshort;
typedef signed int     sint;
typedef signed long    slong;
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef uintptr_t      word;
typedef unsigned char  ubyte;
typedef uint32_t       flags;

typedef char ascii;
typedef char latin1;
typedef char utf8;
typedef char16_t utf16;
typedef char32_t utf32;

typedef int8_t         sint8;
typedef int16_t        sint16;
typedef int32_t        sint32;
typedef int64_t        sint64;
typedef uint8_t        uint8;
typedef uint16_t       uint16;
typedef uint32_t       uint32;
typedef uint64_t       uint64;

typedef size_t         usize;
typedef void          *nullptr;
typedef void         (*funcptr)(void);

#define generic _Generic
#define unlikely(condition) __builtin_expect(0, (condition))
#define unreachable __builtin_unreachable
#define attrs __attribute__

#endif
