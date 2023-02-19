#ifndef value_h
#define value_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef uintptr_t Val;
typedef double Real;
typedef char Glyph;
typedef struct Obj Obj;
typedef struct Sym Sym;
typedef struct Func Func;
typedef struct Bin Bin;
typedef struct List List;
typedef struct Table Table;

typedef union ValData ValData;

union ValData {
  Val     as_val;
  Real    as_real;
  Glyph   as_glyph;
  Obj*    as_obj;
  Sym*    as_sym;
  Func*   as_func;
  Bin*    as_bin;
  List*   as_list;
  Table*  as_table;
};

typedef struct {
  Val *array;
  uint count, cap;
} Vals;

// globals --------------------------------------------------------------------
// tags -----------------------------------------------------------------------
#define QNAN       0x7ff8000000000000ul
#define SIGN       0x8000000000000000ul

#define REAL_TAG    0x0000000000000000ul // dummy tag
#define UNIT_TAG    0x7ffc000000000000ul
#define GLYPH_TAG   0x7ffd000000000000ul
#define OBJ_TAG     0x7ffe000000000000ul

#define TAG_MASK   0xffff000000000000ul
#define VAL_MASK   0x0000fffffffffffful
#define WORD_MASK (TAG_MASK|VAL_MASK)

#define NUL       (UNIT_TAG|UINT32_MAX)

// signal values (invalid objects) --------------------------------------------
#define NOTFOUND  (OBJ_TAG|1)
#define UNDEFINED (OBJ_TAG|2)
#define UNBOUND   (OBJ_TAG|3)

#define TAG_BITS(value) ((value) & TAG_MASK)
#define VAL_BITS(value) ((value) & VAL_MASK)

// tagging methods ------------------------------------------------------------
Val tag_real(Real real);
Val tag_glyph(Glyph glyph);
Val tag_obj(Obj* obj);
Val tag_ptr(void* data);

#define tag(x)					\
  generic((x),					\
	  Real:tag_real,			\
	  FixNum:tag_fixnum,			\
	  int:tag_fixnum,			\
	  uint:tag_fixnum,			\
	  Glyph:tag_glyph,			\
	  Obj*:tag_obj,				\
	  default:tag_ptr)(x)

// untagging methods ----------------------------------------------------------
Real    as_real(Val val);
Glyph   as_glyph(Val val);
Obj*    as_obj(Val val);
Sym*    as_sym(Val val);
Func*   as_func(Val val);
Bin*    as_bin(Val val);
List*   as_list(Val val);
Table*  as_table(Val val);

// generic untagging methods --------------------------------------------------
char* as_text(Val val);
bool  is_int(Real r);

// values array API -----------------------------------------------------------
void init_vals(Vals* vals);
void free_vals(Vals* vals);
void resize_vals(Vals* vals, uint n);
uint push_vals(Vals* vals, Val val);
Val  pop_vals(Vals* vals);

#endif
