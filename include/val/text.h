#ifndef rl_val_text_h
#define rl_val_text_h

#include "val/object.h"

#include "util/text.h"
#include "util/table.h"

/* Types and APIs for Rascal values used in IO, binary data processing, and text representation, as well as supporting globals. */

// user IO types
struct Port {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t input    : 1;
  word_t output   : 1;
  word_t lispfile : 1;

  // data fields
  FILE* ios;

  union {
    Str*  name;
    char* _sname;
  };

  int line;
  int col;

  // reader fields
  // read buffer
  union {
    MStr* t;
    MBin* b;
  } b;

  // last expression read
  Val x;
};

struct Str {
  HEADER;

  // bit fields
  word_t enc      : 4;
  word_t hasmb    : 1;
  word_t interned : 1;
  word_t _static  : 1;

  // data fields
  char*  chars;
  size_t cnt;
};

struct Bin {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  byte*  data;
  size_t cnt;
};

#define DYNAMIC_BUFFER(X)                       \
  word_t algo : 3;                              \
  X* data;                                      \
  X* _static;                                   \
  size_t cnt, maxc, maxs

struct MStr {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;

  // data fields
  DYNAMIC_BUFFER(char);
};

struct MBin {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  DYNAMIC_BUFFER(byte);
};

#undef DYNAMIC_BUFFER

// read table type (intention is for this to eventually be extensible)
#define RT_SIZE 128

struct RT {
  HEADER;

  RT*    parent;
  Func*  eof_fn;
  Func*  dispatch[RT_SIZE];
};

/* Globals */
// types
extern Type PortType, GlyphType, StrType, BinType, MStrType, MBinType, RTType;

/* APIs */
/* Str API */
#define is_str(x) has_type(x, &StrType)
#define as_str(x) ((Str*)as_obj(x))

Str*  mk_str(char* cs, size_t n, bool i);
Str*  get_str(char* cs, size_t n);
Glyph str_ref(Str* s, size_t n);
Str*  str_set(Str* s, size_t n, Glyph g);

/* Bin API */
#define is_bin(x) has_type(x, &BinType)
#define as_bin(x) ((Bin*)as_bin(x))

Bin* mk_bin(size_t n, void* d);
byte bin_ref(Bin* b, size_t n);
Bin* bin_set(Bin* b, size_t n, byte u);

/* RT API */
#define is_rt(x) has_type(x, &RTType)
#define as_rt(x) ((RT*)as_obj(x))

#define rt_set(rt, x, r)                        \
  generic((x),                                  \
          Glyph:rt_set_g,                       \
          int:rt_set_g,                         \
          char*:rt_set_s)(rt, x, r)

RT*    mk_rt(RT* p);
void   init_rt(RT* rt, RT* p);
void   rt_set_g(RT* rt, int d, Func* f);
void   rt_set_s(RT* rt, char* ds, Func* f);
Func*  rt_get(RT* rt, int d);

/* Types and APIs for Rascal values used in IO, binary data processing, and text representation, as well as supporting globals. */

/* Mutable buffer APIs */
#define MUTABLE_BUFFER(T, t, X)                                         \
  T*     new_##t(X* d, size_t n, bool s, ResizeAlgo ag);                \
  void   init_##t(T* a, X* _s, size_t ms, bool s, ResizeAlgo ag);       \
  void   free_##t(void* x);                                             \
  void   grow_##t(T* a, size_t n);                                      \
  void   shrink_##t(T* a, size_t n);                                    \
  size_t write_##t(T* a, X* s, size_t n);                               \
  size_t t##_push(T* a, X x);                                           \
  size_t t##_pushn(T* a, size_t n, ...);                                \
  size_t t##_pushv(T* a, size_t n, va_list va);                         \
  X      t##_pop(T* a);                                                 \
  X      t##_popn(T* a, size_t n, bool e)

MUTABLE_BUFFER(MBin, mbin, byte);
MUTABLE_BUFFER(MStr, mstr, char);

#undef MUTABLE_BUFFER

#endif
