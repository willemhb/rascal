#ifndef value_h
#define value_h

#include <stdio.h>

#include "common.h"

/* C types */
typedef uintptr_t  Val;
typedef double     Real;
typedef uintptr_t  Fix;
typedef int        Small;
typedef bool       Bool;
typedef char       Glyph;
typedef struct Obj Obj;

typedef enum  ValType ValType;
typedef union ValData ValData;

enum ValType {
  REAL_VAL,
  FIX_VAL,
  SMALL_VAL,
  UNIT_VAL,
  BOOL_VAL,
  GLYPH_VAL,
  OBJ_VAL
};

union ValData {
  Val    as_val;
  Real   as_real;
  Fix    as_fix;
  Small  as_small;
  Bool   as_bool;
  Glyph  as_glyph;
  Obj   *as_obj;
};

/* globals */
#define QNAN      0x7ff8000000000000ul
#define SIGN      0x8000000000000000ul

#define REAL_TAG  0x0000000000000000ul
#define FIX_TAG   0x7ffc000000000000ul
#define SMALL_TAG 0x7ffd000000000000ul
#define UNIT_TAG  0x7ffe000000000000ul
#define BOOL_TAG  0x7fff000000000000ul
#define GLYPH_TAG 0xfffc000000000000ul
#define OBJ_TAG   0xfffd000000000000ul

#define TAG_MASK   0xffff000000000000ul
#define VAL_MASK   0x0000fffffffffffful
#define WORD_MASK (TAG_MASK|VAL_MASK)

#define NIL_VAL    UNIT_TAG
#define TRUE_VAL  (BOOL_TAG|1)
#define FALSE_VAL (BOOL_TAG|0)

/* convenience macros */
#define TAG_BITS(value) ((value) & TAG_MASK)
#define VAL_BITS(value) ((value) & VAL_MASK)

/* API */
// general
ValType val_type(Val x);
char *type_name(Val x);
bool is_val_type(Val x, ValType type);

bool is_real(Val x);
bool is_fix(Val x);
bool is_small(Val x);
bool is_unit(Val x);
bool is_bool(Val x);
bool is_glyph(Val x);
bool is_obj(Val x);

Real as_real(Val x);
Fix as_fix(Val x);
Small as_small(Val x);
Bool as_bool(Val x);
Glyph as_glyph(Val x);
Obj *as_obj(Val x);

Val mk_glyph(Glyph g);
Val mk_bool(Bool b);
Val mk_small(Small s);
Val mk_real(Real r);
Val mk_fix(Fix f);
Val mk_obj(Obj *o);

// numbers
bool is_num(Val x);
Fix fix_val(Val x);
Real real_val(Val x);
Small small_val(Val x);

#define mk_val(x)				\
  generic((x),					\
	  Glyph:mk_glyph,			\
	  Bool:mk_bool,				\
	  Small:mk_small,			\
	  Fix:mk_fix,				\
	  Real:mk_real,				\
	  Obj*:mk_obj)(x)

#endif
