#ifndef rascal_common_h
#define rascal_common_h

#include <uchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdalign.h>
#include <setjmp.h>

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
typedef uintptr_t UWord;

typedef Void (*FuncPtr)(Void);

// general convenience 
#define rl_unlikely( test ) __builtin_expect(0, (test))

#define rl_unreachable __builtin_unreachable
#define rl_cleanup( cleaner ) __attribute__((cleanup(cleaner)))


#endif
