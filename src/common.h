#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* style typedefs */
typedef signed char    schar;
typedef signed short   sshort;
typedef signed int     sint;
typedef signed long    slong;
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef unsigned char  ubyte;

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

#endif
