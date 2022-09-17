#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

// style typedefs -------------------------------------------------------------
typedef          char  Char;
typedef signed   char  SChar;
typedef unsigned char  UChar;
typedef          short Short;
typedef signed   short SShort;
typedef unsigned short UShort;
typedef          int   Int;
typedef signed   int   SInt;
typedef unsigned int   UInt;
typedef          long  Long;
typedef signed   long  SLong;
typedef unsigned long  ULong;

typedef float          Float;
typedef double         Double;

typedef void           Void;

typedef int8_t         Int8;
typedef uint8_t        UInt8;
typedef int16_t        Int16;
typedef uint16_t       UInt16;
typedef int32_t        Int32;
typedef uint32_t       UInt32;
typedef int64_t        Int64;
typedef uint64_t       UInt64;

typedef intptr_t       IPtr;
typedef uintptr_t      UPtr;
typedef size_t         Size;
typedef bool           Bool;

// utility typedefs -----------------------------------------------------------
typedef UInt64         Hash;
typedef UInt64         Idno;
typedef UInt32         Arity;

// renames of __builtin -------------------------------------------------------
#define popcount    __builtin_popcount
#define countLz     __builtin_clz
#define countTz     __builtin_ctz
#define unreachable __builtin_unreachable

#endif
