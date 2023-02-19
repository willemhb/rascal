#ifndef base_value_h
#define base_value_h

#include "common.h"

/* C types */
typedef uintptr_t Val;
typedef double Real;
typedef uint64 FixNum;
typedef char Glyph;
typedef struct Obj Obj;
typedef struct Sym Sym;
typedef struct Stream Stream;
typedef struct Func Func;
typedef struct Bin Bin;
typedef struct List List;
typedef struct Table Table;

typedef union ValData ValData;

union ValData {
  char*   as_cstr;
  void*   as_cptr;
  Val     as_val;
  Real    as_real;
  FixNum  as_fixnum;
  Glyph   as_glyph;
  Obj*    as_obj;
  Sym*    as_sym;
  Stream* as_stream;
  Func*   as_func;
  Bin*    as_bin;
  List*   as_list;
  Table*  as_table;
};

/* globals */
#define QNAN       0x7ff8000000000000ul
#define SIGN       0x8000000000000000ul

// used for passing non-lisp data directly to C functions (type is fixnum)
#define SIGNAL_TAG  0x7ffc000000000000ul // used internally to signal errors and conditions
#define CPTR_TAG    0x7ffd000000000000ul // C data not allocated in the heap
#define CSTR_TAG    0x7ffe000000000000ul // nul-terminated C string outside the heap

// value tags
#define REAL_TAG    0x0000000000000000ul // dummy tag
#define FIXNUM_TAG  0x7fff000000000000ul
#define UNIT_TAG    0xfffc000000000000ul
#define GLYPH_TAG   0xfffd000000000000ul
#define OBJ_TAG     0xfffe000000000000ul

#define TAG_MASK   0xffff000000000000ul
#define VAL_MASK   0x0000fffffffffffful
#define WORD_MASK (TAG_MASK|VAL_MASK)

#define NUL       (UNIT_TAG|UINT32_MAX)

#define NOTFOUND  (SIGNAL_TAG|0)
#define UNDEFINED (SIGNAL_TAG|1)
#define UNBOUND   (SIGNAL_TAG|2)
#define PRECEDES  (SIGNAL_TAG|3)
#define EQUALTO   (SIGNAL_TAG|4)
#define EXCEEDS   (SIGNAL_TAG|5)
#define NOTEQUAL  (SIGNAL_TAG|6)

/* API */
#define TAG_BITS(value) ((value) & TAG_MASK)
#define VAL_BITS(value) ((value) & VAL_MASK)

#define tag(x)					\
  generic((x),					\
	  Real:tag_real,			\
	  FixNum:tag_fixnum,			\
	  int:tag_fixnum,			\
	  uint:tag_fixnum,			\
	  Glyph:tag_glyph,			\
	  Obj*:tag_obj,				\
	  Sym*:tag_sym,				\
	  Stream*:tag_stream,			\
	  Func*:tag_func,			\
	  Bin*:tag_bin,				\
	  List*:tag_list,			\
	  Table*:tag_table,			\
	  char*:tag_cstr,			\
	  default:tag_cptr)(x)

// tagging methods ------------------------------------------------------------
Val tag_cptr(void* cptr);
Val tag_cstr(char* cstr);
Val tag_real(Real real);
Val tag_fixnum(FixNum fixnum);
Val tag_glyph(Glyph glyph);
Val tag_obj(Obj* obj);
Val tag_sym(Sym* sym);
Val tag_stream(Stream* stream);
Val tag_func(Func* func);
Val tag_bin(Bin* bin);
Val tag_list(List* list);
Val tag_table(Table* table);

// untagging methods ----------------------------------------------------------
void*   as_cptr(Val val);
char*   as_cstr(Val val);
Real    as_real(Val val);
FixNum  as_fixnum(Val val);
Glyph   as_glyph(Val val);
Obj*    as_obj(Val val);
Sym*    as_sym(Val val);
Stream* as_stream(Val val);
Func*   as_func(Val val);
Bin*    as_bin(Val val);
List*   as_list(Val val);
Table*  as_table(Val val);

// builtin type/tag predicates ------------------------------------------------
bool    is_cptr(Val val);
bool    is_cstr(Val val);
bool    is_real(Val val);
bool    is_fixnum(Val val);
bool    is_glyph(Val val);
bool    is_obj(Val val);

bool    val_is_sym(Val val);
bool    obj_is_sym(Obj* obj);
bool    val_is_stream(Val val);
bool    obj_is_stream(Obj* obj);
bool    val_is_func(Val val);
bool    obj_is_func(Obj* obj);
bool    val_is_bin(Val val);
bool    obj_is_bin(Obj* obj);
bool    val_is_list(Val val);
bool    obj_is_list(Obj* obj);
bool    val_is_table(Val val);
bool    obj_is_table(Obj* obj);

#define is_sym(x)    generic((x), Val:val_is_sym, Obj*:obj_is_sym)(x)
#define is_stream(x) generic((x), Val:val_is_stream, Obj*:obj_is_stream)(x)
#define is_func(x)   generic((x), Val:val_is_func, Obj*:obj_is_func)(x)
#define is_bin(x)    generic((x), Val:val_is_bin, Obj*:obj_is_bin)(x)
#define is_list(x)   generic((x), Val:val_is_list, Obj*:obj_is_list)(x)
#define is_table(x)  generic((x), Val:val_is_table, Obj*:obj_is_table)(x)

char* as_text(Val val);

#endif
