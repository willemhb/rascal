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
  int     line;
  int     col;
  
  flags_t encoding : 28;
  flags_t input    :  1;
  flags_t output   :  1;
  flags_t lispfile :  1;

  // data fields
  FILE* ios;

  union {
    Str*  name;
    char* _sname;
  };

  // reader fields
  // read buffer
  Buffer* b;

  // last expression read
  Val x;
};

struct Bin {
  HEADER;

  // bit fields
  flags_t ctype    : 28;
  flags_t encp     :  1;
  flags_t hasmb    :  1;
  flags_t interned :  1;
  flags_t _static  :  1;

  // data fields
  union {
    char* chars;
    byte* data;
  };

  size_t cnt;
};

struct Buffer {
  HEADER;

  // bit fields
  flags_t ctype    : 27;
  flags_t algo     :  3;
  flags_t encp     :  1;
  flags_t hasmb    :  1;

  // data fields
  union {
    char* chars;
    byte* data;
  };

  size_t cnt, cap;
};

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
#define is_port(x) has_type(x, &PortType)
#define as_port(x) ((Port*)as_obj(x))

#define rl_prf(p, f, ...)                               \
  generic((f),                                          \
          char*:rl_cprf,                                \
          Str*:rl_sprf)(p, f __VA_OPT__(,) __VA_ARGS__)

int rl_cprf(Port* p, char* f, ...);
int rl_sprf(Port* p, Str* f, ...);

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
