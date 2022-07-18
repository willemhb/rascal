#ifndef rascal_common_h
#define rascal_common_h

// common includes ------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// making common C types style compliant

typedef signed char schar;
typedef unsigned char uchar;

typedef signed short   sshort;
typedef unsigned short ushort;

typedef signed int   sint;
typedef unsigned int uint;

typedef signed long   slong;
typedef unsigned long ulong;

// utility typedefs
typedef uint flags_t;
typedef uint arity_t;

typedef ulong hash_t;
typedef ulong index_t;
typedef ulong idno_t;

#endif
