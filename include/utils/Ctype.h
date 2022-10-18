#ifndef rascal_Ctype_h
#define rascal_Ctype_h

#include "common.h"

// C types
typedef enum
  {
    Ctype_sint8,   Ctype_uint8,  Ctype_ascii, Ctype_utf8, Ctype_latin1,

    Ctype_sint16,  Ctype_uint16, Ctype_utf16,

    Ctype_sint32,  Ctype_uint32, Ctype_utf32,   Ctype_float32,

    Ctype_sint64,  Ctype_uint64, Ctype_pointer, Ctype_float64,

    // 
    Ctype_object
  } Ctype_t;

// convenience
static inline size_t Ctype_size( Ctype_t C )
{ 
  switch(C)
    {
    case Ctype_object:                    return 0; // indeterminate size
    case Ctype_sint8  ... Ctype_latin1:   return 1;
    case Ctype_sint16 ... Ctype_utf16:    return 2;
    case Ctype_sint32 ... Ctype_float32:  return 4;
    default:                              return 8;
    }
}

static inline bool is_encoding( Ctype_t C )
{
  switch (C)
    {
    case Ctype_ascii ... Ctype_latin1: return true;
    case Ctype_utf16:                  return true;
    case Ctype_utf32:                  return true;
    default:                           return false;
    }
}

static inline bool is_numeric( Ctype_t C )
{
  switch (C)
    {
    case Ctype_ascii ... Ctype_latin1:
    case Ctype_utf16:
    case Ctype_utf32:
    case Ctype_pointer:
    case Ctype_object:
      return false;

    default:
      return true;
    }
}

static inline bool is_multibyte( Ctype_t C )
{
  return C == Ctype_utf8 || C == Ctype_utf16;
}

static inline Ctype_t enc_numtype( Ctype_t C )
{
  assert(is_encoding(C));

  switch (C)
    {
    case Ctype_utf32: return Ctype_uint32;
    case Ctype_utf16: return Ctype_uint16;
    case Ctype_ascii: return Ctype_sint8;
    default:          return Ctype_uint8;
    }
}

#endif
