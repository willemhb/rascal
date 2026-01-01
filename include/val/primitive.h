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
  union {
    Num    num;
    Bool   boolean;
    Glyph  glyph;
    int    small;
    void*  pointer;
    uptr_t fixnum;
  };
  /*
  if a box represents C data, eg a struct or a string,
  then val.pointer points to data + 8 and the first 8
  bytes of data contains the total allocated size of
  the object. Not currently used but eventually I hope to use
  this for compact representations of binary data (may even
  refactor Str to use this strategy)
  byte_t data[0]; */
};

// boxing API
Box* box_expr(RlState* rls, Expr x);
Box* box_expr_s(RlState* rls, Expr x);
Expr unbox_box(Box* box);
Expr unbox_expr(Expr x);
Box* mk_box(RlState* rls, Type* type, int flags, Expr init);
Box* mk_box_s(RlState* rls, Type* type, int flags, Expr init);

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
