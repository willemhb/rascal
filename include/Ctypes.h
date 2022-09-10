#ifndef rascal_Ctypes_h
#define rascal_Ctypes_h

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef enum
  {
    C_sint8=1, C_uint8,

    C_sint16, C_uint16,

    C_sint32, C_uint32, C_float32,

    C_sint64, C_uint64, C_float64
  } Ctype_t;

typedef enum
  {
    e_ascii=1,
    e_latin1,
    e_utf8,
    e_utf16,
    e_utf32
  } encoding_t;

#endif
