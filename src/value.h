#ifndef value_h
#define value_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef uintptr_t     Val;
typedef double        Real;
typedef uintptr_t     FixNum;
typedef char          Glyph;

typedef struct Obj    Obj;
typedef struct Sym    Sym;
typedef struct Stream Stream;
typedef struct Func   Func;
typedef struct Bin    Bin;
typedef struct Str    Str;
typedef struct List   List;
typedef struct Vec    Vec;
typedef struct Tuple  Tuple;
typedef struct Dict   Dict;
typedef struct Set    Set;
typedef struct Record Record;

typedef struct Chunk  Chunk;
typedef struct UpVal  UpVal;
typedef struct Disp   Disp;
typedef struct Method Method;
typedef struct Cntl   Cntl;

typedef union ValData ValData;

union ValData {
  Val     as_val;
  Real    as_real;
  FixNum  as_fixnum;
  Glyph   as_glyph;
  void*   as_obj;
  void*   as_ptr;
};

// globals --------------------------------------------------------------------
// tags -----------------------------------------------------------------------
#define QNAN        0x7ff8000000000000ul
#define SIGN        0x8000000000000000ul

#define REAL_TAG    0x0000000000000000ul // dummy tag
#define FIXNUM_TAG  0x7ffc000000000000ul
#define UNIT_TAG    0x7ffd000000000000ul
#define GLYPH_TAG   0x7ffe000000000000ul
#define OBJ_TAG     0x7fff000000000000ul
#define SYS_TAG     0xfffc000000000000ul // pointer to C data, don't touch basically

#define TAG_MASK    0xffff000000000000ul
#define VAL_MASK    0x0000fffffffffffful
#define WORD_MASK  (TAG_MASK|VAL_MASK)

#define NUL        (UNIT_TAG|UINT32_MAX)

// signal values (invalid objects) --------------------------------------------
#define NOTFOUND  (OBJ_TAG|1)
#define UNDEFINED (OBJ_TAG|2)
#define UNBOUND   (OBJ_TAG|3)
#define NOTUSED   (OBJ_TAG|4)

#define TAG_BITS(value) ((value) & TAG_MASK)
#define VAL_BITS(value) ((value) & VAL_MASK)

// tagging methods ------------------------------------------------------------
Val tag_real(Real real);
Val tag_fixnum(FixNum fixnum);
Val tag_glyph(Glyph glyph);
Val tag_obj(void* obj);
Val tag_ptr(void* ptr);

#define tag(x)                                  \
  generic((x),                                  \
          Real:tag_real,                        \
          FixNum:tag_fixnum,                    \
          Glyph:tag_glyph,                      \
          Obj*:tag_obj,                         \
          Sym*:tag_obj,                         \
          Func*:tag_obj,                        \
          Bin*:tag_obj,                         \
          Str*:tag_obj,                         \
          List*:tag_obj,                        \
          Vec*:tag_obj,                         \
          Tuple*:tag_obj,                       \
          Dict*:tag_obj,                        \
          Set*:tag_obj,                         \
          Record*:tag_obj,                      \
          UpVal*:tag_obj,                       \
          Disp*:tag_obj,                        \
          Method*:tag_obj,                      \
          Cntl*:tag_obj,                        \
          default:tag_ptr)(x)

// untagging methods ----------------------------------------------------------
Real    as_real(Val val);
Glyph   as_glyph(Val val);
void*   as_ptr(Val val);
void*   as_obj(Val val);
Sym*    as_sym(Val val);
Stream* as_stream(Val val);
Func*   as_func(Val val);
Bin*    as_bin(Val val);
Str*    as_str(Val val);
List*   as_list(Val val);
Vec*    as_vec(Val val);
Tuple*  as_tuple(Val val);
Dict*   as_dict(Val val);
Set*    as_set(Val val);
Record* as_record(Val val);
UpVal*  as_upval(Val val);
Disp*   as_disp(Val val);
Method* as_method(Val val);
Cntl*   as_cntl(Val val);

// type & value predicates ----------------------------------------------------
bool is_text(Val val);
bool is_int(Val val);
bool is_byte(Val val);

// generic untagging methods --------------------------------------------------
char* as_text(Val val);
uint  as_int(Val val);
ubyte as_byte(Val val);

// misc runtime methods -------------------------------------------------------
void mark_val(Val val);

#endif
