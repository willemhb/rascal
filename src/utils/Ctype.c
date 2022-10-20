#include "utils/Ctype.h"

Ctype_t common_Ctype( Ctype_t Cx, Ctype_t Cy )
{
  static const Ctype_t CommonTypes[19][19] =
    {
      [Ctype_sint8] =
      {
	[Ctype_sint8]  = Ctype_sint8,   [Ctype_uint8]  = Ctype_sint16,
	[Ctype_sint16] = Ctype_sint16,  [Ctype_uint16] = Ctype_sint32,
	[Ctype_sint32] = Ctype_sint32,  [Ctype_uint32] = Ctype_sint64,
	[Ctype_float32]= Ctype_float32, [Ctype_sint64] = Ctype_sint64,
	[Ctype_uint64] = Ctype_object,  [Ctype_float64]= Ctype_float64,
	[Ctype_object] = Ctype_object
      },

      [Ctype_uint8] =
      {
	[Ctype_sint8]  = Ctype_sint16,  [Ctype_uint8]  = Ctype_uint8,
	[Ctype_sint16] = Ctype_sint16,  [Ctype_uint16] = Ctype_uint16,
	[Ctype_sint32] = Ctype_sint32,  [Ctype_uint32] = Ctype_uint32,
	[Ctype_float32]= Ctype_float32, [Ctype_sint64] = Ctype_sint64,
	[Ctype_uint64] = Ctype_uint64,  [Ctype_float64]= Ctype_float64,
	[Ctype_object] = Ctype_object
      },

      [Ctype_ascii] =
      {
	[Ctype_ascii]  = Ctype_ascii,   [Ctype_latin1] = Ctype_latin1,
	[Ctype_utf8]   = Ctype_utf8,    [Ctype_utf16]  = Ctype_utf16,
	[Ctype_utf32]  = Ctype_utf32
      },

      [Ctype_utf8] =
      {
	[Ctype_ascii]  = Ctype_utf8,    [Ctype_latin1] = Ctype_utf16,
	[Ctype_utf8]   = Ctype_utf8,    [Ctype_utf16]  = Ctype_utf16,
	[Ctype_utf32]  = Ctype_utf32
      },

      [Ctype_latin1] =
      {
	[Ctype_ascii]  = Ctype_ascii,   [Ctype_latin1] = Ctype_latin1,
	[Ctype_utf8]   = Ctype_utf16,   [Ctype_utf16]  = Ctype_utf16,
	[Ctype_utf32]  = Ctype_utf32
      },

      [Ctype_sint16] =
      {
	[Ctype_sint8]  = Ctype_sint16,  [Ctype_uint8]  = Ctype_sint16,
	[Ctype_sint16] = Ctype_sint16,  [Ctype_uint16] = Ctype_sint32,
	[Ctype_sint32] = Ctype_sint32,  [Ctype_uint32] = Ctype_sint64,
	[Ctype_float32]= Ctype_float32,
	[Ctype_sint64] = Ctype_sint64,  [Ctype_uint64] = Ctype_object,
	[Ctype_float64]= Ctype_float64, [Ctype_object] = Ctype_object
      },

      [Ctype_uint16] =
      {
	[Ctype_sint8]  = Ctype_sint32,  [Ctype_uint8]  = Ctype_uint16,
	[Ctype_sint16] = Ctype_sint32,  [Ctype_uint16] = Ctype_uint16,
	[Ctype_sint32] = Ctype_sint64,  [Ctype_uint32] = Ctype_uint32,
	[Ctype_float32]= Ctype_float32,
	[Ctype_sint64] = Ctype_object,  [Ctype_uint64] = Ctype_uint64,
	[Ctype_float64]= Ctype_float64, [Ctype_object] = Ctype_object
      },

      [Ctype_utf16] =
      {
	[Ctype_ascii]  = Ctype_utf16,   [Ctype_latin1] = Ctype_utf16,
	[Ctype_utf8]   = Ctype_utf16,   [Ctype_utf16]  = Ctype_utf16,
	[Ctype_utf32]  = Ctype_utf32
      },

      [Ctype_sint32] =
      {
	[Ctype_sint8]  = Ctype_sint32,  [Ctype_uint8]  = Ctype_sint32,
	[Ctype_sint16] = Ctype_sint32,  [Ctype_uint16] = Ctype_sint32,
	[Ctype_sint32] = Ctype_sint32,  [Ctype_uint32] = Ctype_sint64,
	[Ctype_float32]= Ctype_float64,
	[Ctype_sint64] = Ctype_sint64,  [Ctype_uint64] = Ctype_object,
	[Ctype_float64]= Ctype_float64, [Ctype_object] = Ctype_object
      },

      [Ctype_uint32] =
      {
	[Ctype_sint8]  = Ctype_sint64,  [Ctype_uint8]  = Ctype_uint32,
	[Ctype_sint16] = Ctype_sint64,  [Ctype_uint16] = Ctype_uint32,
	[Ctype_sint32] = Ctype_sint64,  [Ctype_uint32] = Ctype_uint32,
	[Ctype_float32]= Ctype_float64,
	[Ctype_sint64] = Ctype_object,  [Ctype_uint64] = Ctype_uint64,
	[Ctype_float64]= Ctype_float64, [Ctype_object] = Ctype_object
      },

      [Ctype_float32] =
      {
	[Ctype_sint8]  = Ctype_sint64,  [Ctype_uint8]  = Ctype_uint32,
	[Ctype_sint16] = Ctype_sint64,  [Ctype_uint16] = Ctype_uint32,
	[Ctype_sint32] = Ctype_sint64,  [Ctype_uint32] = Ctype_uint32,
	[Ctype_float32]= Ctype_float64,
	[Ctype_sint64] = Ctype_object,  [Ctype_uint64] = Ctype_uint64,
	[Ctype_float64]= Ctype_float64, [Ctype_object] = Ctype_object
      },
    
      [Ctype_utf32] =
      {
	[Ctype_ascii]  = Ctype_utf32,   [Ctype_latin1] = Ctype_utf32,
	[Ctype_utf8]   = Ctype_utf32,   [Ctype_utf16]  = Ctype_utf32,
	[Ctype_utf32]  = Ctype_utf32
      },

      [Ctype_sint64] =
      {
	[Ctype_sint8]  = Ctype_sint64,  [Ctype_uint8]  = Ctype_sint64,
	[Ctype_sint16] = Ctype_sint64,  [Ctype_uint16] = Ctype_sint64,
	[Ctype_sint32] = Ctype_sint64,  [Ctype_uint32] = Ctype_sint64,
	[Ctype_float32]= Ctype_object,
	[Ctype_sint64] = Ctype_sint64,  [Ctype_uint64] = Ctype_object,
	[Ctype_float64]= Ctype_object,  [Ctype_object] = Ctype_object
      },

      [Ctype_uint64] =
      {
	[Ctype_sint8]  = Ctype_sint64,  [Ctype_uint8]  = Ctype_uint64,
	[Ctype_sint16] = Ctype_sint64,  [Ctype_uint16] = Ctype_uint64,
	[Ctype_sint32] = Ctype_sint64,  [Ctype_uint32] = Ctype_uint64,
	[Ctype_float32]= Ctype_object,
	[Ctype_sint64] = Ctype_object,  [Ctype_uint64] = Ctype_uint64,
	[Ctype_float64]= Ctype_object,  [Ctype_object] = Ctype_object
      },

      [Ctype_float64] =
      {
	[Ctype_sint8]  = Ctype_float64, [Ctype_uint8]  = Ctype_float64,
	[Ctype_sint16] = Ctype_float64, [Ctype_uint16] = Ctype_float64,
	[Ctype_sint32] = Ctype_float64, [Ctype_uint32] = Ctype_float64,
	[Ctype_float32]= Ctype_float64,
	[Ctype_sint64] = Ctype_object,  [Ctype_uint64] = Ctype_object,
	[Ctype_float64]= Ctype_float64, [Ctype_object] = Ctype_object
      },

      [Ctype_string] =
      {
	[Ctype_string] = Ctype_string, [Ctype_object]  = Ctype_object,
	[Ctype_pointer]= Ctype_pointer
      },

      [Ctype_object] =
      {
	[Ctype_string] = Ctype_object, [Ctype_object]  = Ctype_object,
	[Ctype_pointer]= Ctype_pointer,
      },

      [Ctype_pointer] =
      {
	[Ctype_string] =Ctype_pointer, [Ctype_object]  = Ctype_pointer,
	[Ctype_pointer]=Ctype_pointer
      }
    };
  
  return CommonTypes[Cx][Cy];
}
