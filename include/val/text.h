#ifndef rl_text_h
#define rl_text_h

#include "val/object.h"

#include "util/text.h"

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

struct String {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;

  // data fields
  char*  chars;
  size_t count;
};

struct Binary {
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

struct MutStr {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;

  // data fields
  DYNAMIC_BUFFER(char);
};

struct MutBin {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  DYNAMIC_BUFFER(uint8_t);
};

#undef DYNAMIC_BUFFER

// read table type (intention is for this to eventually be extensible)
struct ReadTable {
  HEADER;

  ReadTable* parent;
  funcptr_t  dispatch[256]; // common readers
  funcptr_t  intrasym[256]; // intra-symbol readers
};

/* Globals */
// types
extern Type PortType, StringType, BinaryType, MutStrType, MutBinType, ReadTableType;

// standard ports
extern Port StdIn, StdOut, StdErr;

// empty string
extern String EmptyString;

// string cache
extern StrCache StringCache;

/* APIs */
// string API
String* get_str(char* chars);
String* new_str();

// binary API
Binary* new_bin(size_t n, void* d, CType ct);

// buffer APIs
#define MUTABLE_BUFFER(T, t, X)                                         \
  T*          new_##t(size_t n, X* d, CType ct);                        \
  rl_status_t init_##t(T* a, X* s, size_t ss, CType ct);                \
  rl_status_t free_##t(T* a);                                           \
  rl_status_t grow_##t(T* a, size_t n);                                 \
  rl_status_t shrink_##t(T* a, size_t n);                               \
  rl_status_t write_##t(T* a, X* s, size_t n);                          \
  rl_status_t t##_push(T* a, X x);                                      \
  rl_status_t t##_pushn(T* a, size_t n, ...);                           \
  rl_status_t t##_pushv(T* a, size_t n, va_list va);                    \
  rl_status_t t##_pop(T* a, X* r);                                      \
  rl_status_t t##_popn(T* a, X* r, bool e, size_t n)

MUTABLE_BUFFER(MutBin, mbin, uint8_t);
MUTABLE_BUFFER(MutStr, mstr, char);

#undef MUTABLE_BUFFER

#endif
