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
  
  flags32 encoding : 28;
  flags32 input    :  1;
  flags32 output   :  1;
  flags32 lispfile :  1;

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
  flags32 ctype    : 25;
  flags32 algo     :  3;
  flags32 encp     :  1;
  flags32 hasmb    :  1;
  flags32 interned :  1;
  flags32 _static  :  1;

  union {
    size64 arity;

    struct {
      size32 cnt;
      size32 cap;
    };
  };

  // data fields
  union {
    char* chars;
    byte* data;
  };
};

/* Globals */
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

#endif
