#ifndef rl_primitive_h
#define rl_primitive_h

#include "val/val.h"

typedef union {
  Num    num;
  Bool   boolean;
  Glyph  glyph;
  int    small;
  void*  pointer;
  uptr_t fixnum;
} CData;

struct Box {
  HEAD;

  CData val;
};

// number API
Num as_num_s(RlState* rls, Expr x);
Num as_num(Expr x);
Expr tag_num(Num n);
uintptr_t as_fix(Expr x);
Expr tag_fix(uintptr_t i);
void* as_ptr(Expr x);
Expr tag_ptr(void* ptr);

// boolean API
Bool as_bool(Expr x);
Expr tag_bool(Bool b);

// glyph API
char* char_name(Glyph g);
Glyph as_glyph(Expr x);
Glyph as_glyph_s(RlState* rls, Expr x);
Expr  tag_glyph(Glyph x);

#endif
