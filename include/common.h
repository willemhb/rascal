#ifndef rascal_common_h
#define rascal_common_h

// common includes ------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// making common C types style compliant
typedef void void_t;
typedef bool bool_t;

typedef char        char_t;
typedef signed char schar_t;
typedef unsigned char uchar_t;

typedef short          short_t;
typedef signed short   sshort_t;
typedef unsigned short ushort_t;

typedef int          int_t;
typedef signed int   sint_t;
typedef unsigned int uint_t;

typedef long          long_t;
typedef signed long   slong_t;
typedef unsigned long ulong_t;
typedef float float_t;
typedef double double_t;

// utility typedefs
typedef ulong_t arity_t;
typedef ulong_t hash_t;
typedef ulong_t idno_t;
typedef ulong_t index_t;

#endif
