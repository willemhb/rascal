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
typedef UInt32         Flags;

// renames of __builtin -------------------------------------------------------
#define popcount(x)				\
  _Generic((x),					\
	   UInt32: __builtin_popcount,		\
	   UInt64: __builtin_popcountl)(x)



#define countLz(x)				\
  _Generic((x),					\
	   UInt32: __builtin_clz,		\
	   UInt64: __builtin_clzl)(x)

#define countTz(x)				\
  _Generic((x),					\
	   UInt32: __builtin_ctz,		\
	   UInt64: __builtin_ctzl)(x)

#define unreachable __builtin_unreachable
#define attribute   __attribute__

#define unlikely(x) __builtin_expect(0,(x))
#define likely(x)   __builtin_expect(1,!!(x))

// common describe macros -----------------------------------------------------
#define FlagPredicate(flag, FLAG)		\
  Bool is##flag##Fl( Flags fl )			\
  {						\
    return !!(fl&FLAG);				\
  }

#endif
