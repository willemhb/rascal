#ifndef value_h
#define value_h

#include "common.h"

/* C types */
typedef uintptr_t Val;
typedef double Real;
typedef sint32 Int;
typedef bool Bool;
typedef char Glyph;
typedef struct Obj Obj;
typedef struct Sym Sym;
typedef struct Stream Stream;
typedef struct Func Func;
typedef struct Tuple Tuple;
typedef struct Bin Bin;
typedef struct List List;
typedef struct Vec Vec;
typedef struct Table Table;

typedef union ValData ValData;

union ValData {
  Val     as_val;
  Real    as_real;
  Int     as_int;
  Bool    as_bool;
  Glyph   as_glyph;
  Obj*    as_obj;
  Bin*    as_bin;
  Sym*    as_sym;
  Stream* as_stream;
  Func*   as_func;
  Tuple*  as_tuple;
  List*   as_list;
  Vec*    as_vec;
  Table*  as_table;
};

/* globals */
#define QNAN       0x7ff8000000000000ul
#define SIGN       0x8000000000000000ul

#define REAL_TAG   0x0000000000000000ul
#define UNIT_TAG   0x7ffc000000000000ul
#define BOOL_TAG   0x7ffd000000000000ul
#define GLYPH_TAG  0x7ffe000000000000ul
#define OBJ_TAG    0x7fff000000000000ul

#define TAG_MASK   0xffff000000000000ul
#define VAL_MASK   0x0000fffffffffffful
#define WORD_MASK (TAG_MASK|VAL_MASK)

/* convenience macros */
#define TAG_BITS(value) ((value) & TAG_MASK)
#define VAL_BITS(value) ((value) & VAL_MASK)

/* API */

#endif
