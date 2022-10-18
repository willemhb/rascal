#ifndef rascal_repr_h
#define rascal_repr_h

#include "common.h"
#include "utils/Ctype.h"

// typedefs for all base object types, plus some utility types and macros
// object layouts (unless < CVALUE, corresponds to a concrete type)

// word types
typedef uintptr_t          value_t;  // tagged
typedef uintptr_t          data_t;   // untagged

// object types
// base object types
typedef struct object_t    object_t;
typedef struct boxed_t     boxed_t;

// boxed types
typedef struct record_t    record_t;
typedef struct table_t     table_t;
typedef struct array_t     array_t;
typedef struct hamt_t      hamt_t;
typedef struct amt_t       amt_t;
typedef struct cvalue_t    cvalue_t;
typedef struct type_t      type_t;

// primitive object types
typedef struct symbol_t    symbol_t;
typedef struct cons_t      cons_t;
typedef struct pair_t      pair_t;
typedef struct function_t  function_t;
typedef struct stream_t    stream_t;
typedef struct control_t   control_t;

// table entry object types
typedef struct variable_t  variable_t;
typedef struct slot_t      slot_t;
typedef struct upvalue_t   upvalue_t;

// immediate types
// special immediates
typedef uintptr_t          nul_t;
typedef bool               boolean_t;
typedef enum repr_t        repr_t;
typedef uint16_t           opcode_t;
typedef uint16_t           primitive_t;

// character types
typedef char               ascii_t;
typedef uchar              latin1_t;
typedef uchar              utf8_t;
typedef char16_t           utf16_t;
typedef char32_t           utf32_t;

// small numbers
typedef int8_t             sint8_t;
typedef uint8_t            uint8_t;
typedef int16_t            sint16_t;
typedef uint16_t           uint16_t;
typedef int32_t            sint32_t;
typedef uint32_t           uint32_t;
typedef float              real32_t;

// big immediates
typedef double             real_t;
typedef uint64_t           fixnum_t;
typedef void              *pointer_t;

enum repr_t
  {
    // boxed types
    RECORD, TABLE, ARRAY, HAMT, AMT, CVALUE, TYPE,

    // primitive types
    SYMBOL, CONS, PAIR, FUNCTION, STREAM, CONTROL,

    // internal types (mostly different binding types)
    VARIABLE, SLOT, UPVALUE,

    // special immediates
    NUL, BOOLEAN, REPR, CTYPE, OPCODE, PRIMITIVE,

    // character immediates
    ASCII, LATIN1, UTF8, UTF16, UTF32,

    // small numeric immediates
    SINT8, UINT8, SINT16, UINT16, SINT32, UINT32, REAL32,

    // big numeric immediates
    FIXNUM, REAL,

    // other large immediates
    POINTER,
  };

#define N_REPR (POINTER+1)

static inline size_t repr_size( repr_t r )
{
  switch(r)
    {
    case RECORD ... NUL: return 0;
    case BOOLEAN:        return sizeof(bool);
    case REPR:           return sizeof(repr_t);
    case CTYPE:          return sizeof(Ctype_t);
    case OPCODE:         return sizeof(opcode_t);
    case PRIMITIVE:      return sizeof(primitive_t);
    case ASCII:          return sizeof(ascii_t);
    case LATIN1:         return sizeof(latin1_t);
    case UTF8:           return sizeof(utf8_t);
    case UTF16:          return sizeof(utf16_t);
    case UTF32:          return sizeof(utf32_t);
    case SINT8:          return sizeof(sint8_t);
    case UINT8:          return sizeof(uint8_t);
    case SINT16:         return sizeof(sint16_t);
    case UINT16:         return sizeof(uint16_t);
    case SINT32:         return sizeof(sint32_t);
    case UINT32:         return sizeof(uint32_t);
    case REAL32:         return sizeof(real32_t);
    case FIXNUM:         return sizeof(fixnum_t);
    case REAL:           return sizeof(real_t);
    case POINTER:        return sizeof(pointer_t);
    }
}

static inline Ctype_t repr_Ctype( repr_t r )
{
  switch (r)
    {
    case RECORD ... NUL: return Ctype_object;
    case BOOLEAN:        return Ctype_sint32;
    case REPR:
    case CTYPE:          return Ctype_sint32;
    case OPCODE:
    case PRIMITIVE:      return Ctype_uint16;
    case ASCII:          return Ctype_ascii;
    case LATIN1:         return Ctype_latin1;
    case UTF8:           return Ctype_utf8;
    case UTF16:          return Ctype_utf16;
    case UTF32:          return Ctype_utf32;
    case SINT8:          return Ctype_sint8;
    case UINT8:          return Ctype_uint8;
    case SINT16:         return Ctype_sint16;
    case UINT16:         return Ctype_uint16;
    case SINT32:         return Ctype_sint32;
    case UINT32:         return Ctype_uint32;
    case REAL32:         return Ctype_float32;
    case FIXNUM:         return Ctype_uint64;
    case REAL:           return Ctype_float64;
    case POINTER:        return Ctype_pointer;
    }
}

#endif
