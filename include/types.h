#ifndef rascal_types_h
#define rascal_types_h

#include <stdio.h>

#include "strutils.h"
#include "numutils.h"

#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

#define tag_fixnum     0b000
#define tag_immediate  0b001
#define tag_pair       0b010
#define tag_boxed      0b011

#define flag_gray     0b100
#define flag_proper   0b100

/* core types and reprs

   a repr (short for "representation") is the concrete runtime object that represents a value.

   the purpose of reprs is to decouple the type system from the runtime management of objects.

   two values of the same type can have different 
   
   all values have one of the following "reprs": fixnum, immediate, pair, symbol, function, tuple, binary, table

   
   
   typedef struct {
       value_t car;
       value_t cdr;
   } pair_t;

   typedef struct {
       type_t  type;
       flags_t flags;
       value_t bind;
       hash_t  hash;
       idno_t  idno;
       char name[0];
   } symbol_t;

   // 
   typedef struct {
       type_t   type;
       flags_t  flags;
       value_t *data;

       union {
          size_t   length;
	  table_t *map;    // slot map
       };

       union {
           value_t parent;
	   value_t space[0];
       };
   } tuple_t;

   typedef enum {
     C_sint8,  C_uint8,  C_ascii, C_utf8,    C_latin1,

     C_sint16, C_uint16, C_utf16, C_float16,

     C_sint32, C_uint32, C_utf32, C_ptr32,   C_float32,  C_imag32,

     C_sint64, C_uint64, C_ptr64, C_float64, C_imag64
   } Ctype_t;

   typedef struct {
   // type for storing binary data
   // basis for user types that can be unboxed
   type_t     type;
   flags_t    flags;
       
   union {
   uchar      *data;      // any non-primitives
   
   void       *p64val;
   int64_t     s64val;
   uint64_t    u64val;
   double      f64val;
   imag double i64val;
   
   int32_t     s32val;
   uint32_t    u32val;
   char32_t    c32val;
   float       f32val;
   imag float  i32val;
   
   int16_t     s16val;
   uint16_t    u16val;
   char16_t    c16val;
   short float f16val;
   
   int8_t      s8val;
   uint8_t     u8val;
   char8_t     c8val;
   };
   
   union {
   size_t    length; // element count
   tuple_t  *dim;    // array dimensions
   table_t  *map;    // structure map
   hash_t    hash;   // primitives use this space to cache their hash
   };
   
   union {
   uchar space[0];
   value_t parent;
   };
   } binary_t;

   typedef struct {
       type_t  type;
       flags_t flags;

       value_t name;       // the name of this function if any
       value_t signature;  // tuple of types in function signature

       union {
         value_t (*builtin)( index_t base, size_t argc ); // builtin function
	 value_t template;                                // arbitrary function information
       };
   } function_t;

   typedef struct {
       type_t   type;
       uchar    flags;
       uchar    depth;
       ushort   size;
       value_t *data;
       
       uint bmap, children;
       
   } table_t;

   typedef struct {
       tuple_t base;
       value_t count;
       value_t table;
       value_t entries;
       value_t signature;
   } dict_t;

 */


typedef enum
  {
    type_fixnum    = 0x00|tag_fixnum,

    /* miscellaneous internal types */
    type_type      = 0x00|tag_immediate,
    type_boolean   = 0x08|tag_immediate,
    type_character = 0x10|tag_immediate,

    /* list types */
    type_cons      = 0x00|tag_pair,
    type_nil       = 0x08|tag_pair,

    /* function types */
    type_builtin   = 0x00|tag_boxed,
    type_closure   = 0x08|tag_boxed,

    /* symbol types */
    type_symbol    = 0x10|tag_boxed,

    /* array types */
    type_vector    = 0x18|tag_boxed,
    type_string    = 0x20|tag_boxed,
    type_binary    = 0x28|tag_boxed,

    /* mapping types */
    type_dict      = 0x30|tag_boxed,
    type_set       = 0x38|tag_boxed
  } type_t;

#define wtag_mask 0xff000003
#define tag_mask  0x3

static const value_t tag_type      = (type_type<<24)|tag_immediate;
static const value_t tag_boolean   = (type_boolean<<24)|tag_immediate;
static const value_t tag_character = (type_character<<24)|tag_immediate;

static const value_t tag_builtin   = (type_builtin<<3)|tag_type;
static const value_t tag_closure   = (type_closure<<3)|tag_type;
static const value_t tag_symbol    = (type_symbol<<3)|tag_type;
static const value_t tag_vector    = (type_vector<<3)|tag_type;
static const value_t tag_binary    = (type_binary<<3)|tag_type;
static const value_t tag_string    = (type_string<<3)|tag_type;
static const value_t tag_dict      = (type_dict<<3)|tag_type;
static const value_t tag_set       = (type_set<<3)|tag_type;

static const value_t val_nil     = (type_nil<<24)|tag_immediate;
static const value_t val_true    = (type_boolean<<24)|(1<<3)|tag_immediate;
static const value_t val_false   = (type_boolean<<24)|tag_immediate;
static const value_t val_evec    = (type_vector<<24)|tag_immediate;
static const value_t val_estr    = (type_string<<24)|tag_immediate;
static const value_t val_edict   = (type_dict<<24)|tag_immediate;
static const value_t val_eset    = (type_set<<24)|tag_immediate;
static const value_t val_zero    = (0ul<<3)|tag_fixnum;
static const value_t val_onem    = (((value_t)-1l)<<3)|tag_fixnum;
static const value_t val_one     = (1ul<<3)|tag_fixnum;

static const value_t val_ebin_s8  = (type_binary<<24)|(C_sint8<<3)|tag_immediate;
static const value_t val_ebin_u8  = (type_binary<<24)|(C_uint8<<3)|tag_immediate;
static const value_t val_ebin_s16  = (type_binary<<24)|(C_sint16<<3)|tag_immediate;
static const value_t val_ebin_u16  = (type_binary<<24)|(C_uint16<<3)|tag_immediate;
static const value_t val_ebin_s32  = (type_binary<<24)|(C_sint32<<3)|tag_immediate;
static const value_t val_ebin_u32  = (type_binary<<24)|(C_uint32<<3)|tag_immediate;
static const value_t val_ebin_s64  = (type_binary<<24)|(C_sint64<<3)|tag_immediate;
static const value_t val_ebin_f64  = (type_binary<<24)|(C_float64<<3)|tag_immediate;

static const value_t val_unbound = (type_symbol<<24)|tag_immediate;
static const value_t val_forward = (type_builtin<<24)|tag_immediate;

#define type_pad 128

typedef struct {
  type_t  type;
  flags_t Ctype         :  3;
  flags_t encoding      :  3;
  flags_t is_array      :  1;
  flags_t is_traversed  :  1;
  flags_t flags         :  8;
  flags_t base_size     : 16;
} header_t;

typedef struct {
  header_t base;
  uchar space[0];
} object_t;

typedef struct {
  header_t base;
  size_t   size;
  value_t *data;
  size_t   length;
} vector_t;

typedef struct {
  header_t base;
  size_t   size;
  uchar   *data;
  size_t   length;
} binary_t;

typedef struct {
  header_t base;
  size_t   size;
  char    *data;
  size_t   length;
} string_t;

typedef struct {
  header_t base;
  void (*callback)( size_t n );
} builtin_t;

typedef struct {
  header_t base;
  value_t  envt;
  value_t  vals;
  value_t  code;
} closure_t;

typedef struct {
  header_t base;
  value_t  bind;
  idno_t   idno;
  hash_t   hash;
  char     name[0];
} symbol_t;

typedef long fixnum_t;
typedef bool boolean_t;
typedef char character_t;

typedef struct {
  value_t car;
  value_t cdr;
} cons_t;

typedef struct symbols_t {
  struct symbols_t *left, *right;
  symbol_t base;
} symbols_t;

typedef struct {
  header_t  base;
  ushort    depth, size, bmap, children;

  cons_t  *data;
  value_t  cache;
} table_t;

typedef table_t dict_t;
typedef table_t set_t;

#define uval(x)       ((x)>>3)
#define ival(x)       ((long)uval(x))
#define cval(x)       (uval(x)&0x1fffff)
#define pval(x)       ((void*)(((value_t)(x))&~7ul))

#define tag(x)        ((x)&7)
#define wtag(x)       ((x)&wtag_mask)
#define tagp(p, t)    ((((value_t)(p))&~7ul)|(t))
#define tagi(i, t)    ((((value_t)(i))<<3)|(t))

#define fix_1 8

// global type information tables ---------------------------------------------
extern char    *Typenames[type_pad];

extern size_t   (*sizeof_dispatch[type_pad])(value_t x);
extern size_t   (*prin_dispatch[type_pad])(FILE *ios, value_t x);
extern int      (*order_dispatch[type_pad])(value_t x, value_t y);
extern hash_t   (*hash_dispatch[type_pad])(value_t x);

extern value_t  empty_bins[C_float64+1];

#endif
