#include "def/vmtypes.h"
#include "rascal.h"

/* commentary */

/* C types */

/* globals */
vmtype_spec_t TypeSpecs[] =
  {
    [vmtype_ascii] =
    {
      .name="ascii",
      .size=sizeof(ascii_t),
      .align=alignof(ascii_t),
      .numtype=numtype_unsigned,
      .chartype=chartype_ascii
    },

    [vmtype_latin1] =
    {
      .name="latin1",
      .size=sizeof(latin1_t),
      .align=alignof(latin1_t),
      .numtype=numtype_unsigned,
      .chartype=chartype_latin1
    },

    [vmtype_utf8] =
    {
      .name="utf-8",
      .size=sizeof(utf8_t),
      .align=alignof(utf8_t),
      .numtype=numtype_unsigned,
      .chartype=chartype_utf8
    },

    [vmtype_utf16] =
    {
      .name="utf-16",
      .size=sizeof(utf16_t),
      .align=alignof(utf16_t),
      .numtype=numtype_unsigned,
      .chartype=chartype_utf16
    },

    [vmtype_utf32] =
    {
      .name="utf-32",
      .size=sizeof(utf32_t),
      .align=alignof(utf32_t),
      .numtype=numtype_unsigned,
      .chartype=chartype_utf32
    },

    [vmtype_sint8] =
    {
      .name="sint-8",
      .size=sizeof(int8_t),
      .align=alignof(int8_t),
      .numtype=numtype_signed
    },

    [vmtype_sint16] =
    {
      .name="sint-16",
      .size=sizeof(int16_t),
      .align=alignof(int16_t),
      .numtype=numtype_signed
    },

    [vmtype_sint32] =
    {
      .name="sint-32",
      .size=sizeof(int32_t),
      .align=alignof(int32_t),
      .numtype=numtype_signed
    },

    [vmtype_sint64] =
    {
      .name="sint-64",
      .size=sizeof(int64_t),
      .align=alignof(int64_t),
      .numtype=numtype_signed
    },

    [vmtype_uint8] =
    {
      .name="uint-8",
      .size=sizeof(uint8_t),
      .align=alignof(uint8_t),
      .numtype=numtype_unsigned
    },

    [vmtype_uint16] =
    {
      .name="uint-16",
      .size=sizeof(uint16_t),
      .align=alignof(uint16_t),
      .numtype=numtype_unsigned
    },

    [vmtype_uint32] =
    {
      .name="uint-32",
      .size=sizeof(uint32_t),
      .align=alignof(uint32_t),
      .numtype=numtype_unsigned
    },

    [vmtype_uint64] =
    {
      .name="uint-64",
      .size=sizeof(uint64_t),
      .align=alignof(uint64_t),
      .numtype=numtype_unsigned
    },

    [vmtype_flo32] =
    {
      .name="float-32",
      .size=sizeof(float),
      .align=alignof(float),
      .numtype=numtype_float
    },

    [vmtype_flo64]  =
    {
      .name="float-64",
      .size=sizeof(double),
      .align=alignof(double),
      .numtype=numtype_float
    },

    [vmtype_nulptr] =
    {
      .name="nulptr",
      .size=sizeof(nullptr_t),
      .align=alignof(nullptr_t),
      .numtype=numtype_unsigned
    },

    [vmtype_stream] =
    {
      .name="stream",
      .size=sizeof(stream_t),
      .align=alignof(stream_t),
      .numtype=numtype_unsigned
    },

    [vmtype_funptr] =
    {
      .name="c-function",
      .size=sizeof(funcptr),
      .align=alignof(funcptr),
      .numtype=numtype_unsigned
    },
    
    [vmtype_value] =
    {
      .name="value",
      .size=sizeof(value_t),
      .align=alignof(value_t),
      .numtype=numtype_unsigned
    },

    /* common object case */
    [vmtype_array]  =
    {
      .name="array",
      .size=sizeof(object_t),
      .align=alignof(object_t),
      .numtype=numtype_unsigned
    },

    [vmtype_string] =
    {
      .name="string",
      .size=sizeof(object_t),
      .align=alignof(object_t),
      .numtype=numtype_unsigned
    },

    [vmtype_table] =
    {
      .name="table",
      .size=sizeof(object_t),
      .align=alignof(object_t),
      .numtype=numtype_unsigned
    },

     [vmtype_record] =
    {
      .name="record",
      .size=sizeof(object_t),
      .align=alignof(object_t),
      .numtype=numtype_unsigned
    }
  };

/* API */

/* convenience */
