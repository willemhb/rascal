#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"
#include "utils/Ctype.h"

// common utilities and base typedefs go here
typedef uintptr_t value_t;    // tagged
typedef uintptr_t data_t;     // untagged

typedef struct type_t type_t; // type information
// common object type
typedef struct
{
  union
  {
    type_t *dtype;             // the type is always tagged, but this aids static initialization
    value_t type;              // tagged pointer to type object

    struct
    {
      value_t permanent   :  1; // object is never collected (but may need to be freed at exit)
      value_t allocated   :  1; // object lives in heap (some objects, eg builtin types, symbol table, &c, live in static memory)
      value_t notrace     :  1; // skip tracing this object (mostly exists to avoid tracing symbol table)
      value_t             : 45;
      value_t black       :  1; // live?
      value_t gray        :  1; // traced?
      value_t             : 14;
    };
  };
  byte space[0];
} object_t;

#define OBJECT object_t obj;

// allocated object type (invasive linked list of live objects before header)
typedef struct heap_object_t
{
  struct heap_object_t *next;
  object_t              obj;
} heap_object_t;

typedef struct type_t type_t;

// immediate types
typedef int8_t    sint8_t;
typedef char      ascii_t;
typedef char8_t   latin1_t;
typedef char8_t   utf8_t;
typedef int16_t   sint16_t;
typedef char16_t  utf16_t;
typedef uint16_t  opcode_t;
typedef uint16_t  primitive_t;
typedef int32_t   sint32_t;
typedef char32_t  utf32_t;
typedef float     real32_t;
typedef bool      boolean_t;
typedef void     *nul_t;
typedef void     *pointer_t;
typedef uint64_t  fixnum_t;
typedef double    real_t;

// union of all value representations
typedef union
{
  data_t      as_data;
  value_t     as_value;
  object_t   *as_object;
  type_t     *as_type;

  sint8_t     as_sint8;
  uint8_t     as_uint8;
  ascii_t     as_ascii;
  latin1_t    as_latin1;
  utf8_t      as_utf8;

  sint16_t    as_sint16;
  uint16_t    as_uint16;
  utf16_t     as_utf16;
  opcode_t    as_opcode;
  primitive_t as_primitive;

  sint32_t    as_sint32;
  uint32_t    as_uint32;
  utf32_t     as_utf32;
  real32_t    as_real32;
  Ctype_t     as_Ctype;
  boolean_t   as_boolean;

  nul_t       as_nul;
  pointer_t  *as_pointer;
  char       *as_Cstring;
  fixnum_t    as_fixnum;
  real_t      as_real;
} rl_value_t;

// tags
#define QNAN_BITS 32764
#define SIGN_BIT  32768

#define QNAN      ((value_t)QNAN_BITS<<48)
#define SIGN      ((value_t)SIGN_BIT<<48)

#define IMM_TAG   ((value_t)(QNAN_BITS|0)<<48)
#define PTR_TAG   ((value_t)(QNAN_BITS|1)<<48)
#define FIX_TAG   ((value_t)(QNAN_BITS|2)<<48)
#define OBJ_TAG   ((value_t)(QNAN_BITS|3)<<48)

#define HDR_TAG   (QNAN|SIGN)

#define NUL_VAL      (IMM_TAG|((value_t)NUL<<32))
#define EOS_VAL      (IMM_TAG|((value_t)ASCII<<32)|(value_t)EOF)
#define TRUE_VAL     (IMM_TAG|((value_t)BOOLEAN<<32)|(value_t)1)
#define FALSE_VAL    (IMM_TAG|((value_t)BOOLEAN<<32)|(value_t)0)

#define VT_MASK       (OBJ_TAG|SIGN)
#define IT_MASK       ((value_t)UINT32_MAX<<32)
#define IMM_MASK      (UINT32_MAX)
#define PTR_MASK      (~VT_MASK)
#define HDR_MASK      (PTR_MASK&(~((value_t)7)))

// forward declarations
void     *val_as_ptr( value_t x );
object_t *obj_as_ptr( object_t *obj );
void     *ptr_as_ptr( void *p );
void      val_mark( value_t val );

// convenience
#define val_tag(x)   ((x)&VT_MASK)
#define imm_tag(x)   ((x)>>32&255)

#define imm_data(x)  ((x)&IT_MASK)
#define ptr_data(x)  ((x)&PTR_MASK)
#define fix_data(x)  ((x)&PTR_MASK)
#define real_data(x) (x)
#define obj_data(x)  ((x)&PTR_MASK)
#define hdr_data(x)  ((x)&HDR_MASK)

#define as_fix(x)     fix_data(x)
#define as_real(x)    (((rl_value_t)(x)).as_real)
#define as_boolean(x) ((x)==TRUE_VAL)
#define as_nul(x)     NULL

#define as_ptr(x)							\
  _Generic((x),								\
	   value_t:val_as_ptr,						\
	   object_t*:obj_as_ptr,					\
	   default:ptr_as_ptr)((x))

#define as_obj(x) ((object_t*)as_ptr(x))

static inline bool is_imm( value_t val )  { return (val&VT_MASK) == IMM_TAG; }
static inline bool is_fix( value_t val )  { return (val&VT_MASK) == FIX_TAG; }
static inline bool is_ptr( value_t val )  { return (val&VT_MASK) == PTR_TAG; }
static inline bool is_hdr( value_t val )  { return (val&VT_MASK) == HDR_TAG; }
static inline bool is_obj( value_t val )  { return (val&VT_MASK) == OBJ_TAG; }
static inline bool is_real( value_t val ) { return (val&QNAN) != QNAN; }

#endif
