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
};

struct Str {
  HEADER;

  // bit fields
  word_t enc   : 4;
  word_t hasmb : 1;

  // data fields
  char*  chars;
  size_t count;
};

struct Bin {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  byte_t* data;
  size_t  count;
};

#define DYNAMIC_BUFFER(X)                       \
  word_t algo : 1;                              \
  X* data;                                      \
  X* _static;                                   \
  size_t count, max_count, max_static

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
  DYNAMIC_BUFFER(uint8_t);
};

#undef DYNAMIC_BUFFER

// read table type (intention is for this to eventually be extensible)
struct RT {
  HEADER;

  RT*       parent;
  funcptr_t dispatch[256]; // common readers
  funcptr_t intrasym[256]; // intra-symbol readers
};

/* Globals */
// types
extern Type PortType, StrType, BinType, MStrType, MBinType, RTType;

// standard ports
extern Port StdIn, StdOut, StdErr;

// string cache
extern SCache StrCache;

/* APIs */
// string API
#define is_str(x) has_type(x, &StrType)
#define as_str(x) ((Str*)as_obj(x))

Str*  get_str(char* cs, size_t n);
Str*  new_str(char* cs, size_t n);
Glyph str_ref(Str* s, size_t n);
Str*  str_set(Str* s, size_t n, Glyph g);

// binary API
Bin* new_bin(size_t n, void* d, CType ct);

// buffer APIs
#define MUTABLE_BUFFER(T, t, X)                                         \
  T*     new_##t(X* d, size_t n, bool s, CType ct, ResizeAlgo ag);      \
  void   init_##t(T* a, X* _s, size_t ms, bool s, CType ct, ResizeAlgo ag); \
  void   free_##t(void* x);                                             \
  void   grow_##t(T* a, size_t n);                                      \
  void   shrink_##t(T* a, size_t n);                                    \
  size_t write_##t(T* a, X* s, size_t n);                               \
  size_t t##_push(T* a, X x);                                           \
  size_t t##_pushn(T* a, size_t n, ...);                                \
  size_t t##_pushv(T* a, size_t n, va_list va);                         \
  X      t##_pop(T* a);                                                 \
  X      t##_popn(T* a, size_t n, bool e)

MUTABLE_BUFFER(MBin, mbin, uint8_t);
MUTABLE_BUFFER(MStr, mstr, char);

#undef MUTABLE_BUFFER

#endif
