#ifndef rascal_common_h
#define rascal_common_h
#include <uchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdalign.h>

// style typedefs
typedef uint8_t  Byte;
typedef char     Char;
typedef short    Short;
typedef int      Int;
typedef long     Long;

typedef unsigned char  UChar;
typedef unsigned short UShort;
typedef unsigned int   UInt;
typedef unsigned long  ULong;

typedef float Float;
typedef double Double;

typedef void Void;

typedef size_t    Size;
typedef bool      Bool;
typedef intptr_t  Word;
typedef intptr_t  UWord;

// general convenience 
#define unlikely(test) __builtin_expect(0, (test))

#define unreachable __builtin_unreachable

#endif
