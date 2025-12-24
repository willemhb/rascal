#include "val/primitive.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"

// forward declarations
void print_num(Port* ios, Expr x);
void print_bool(Port* ios, Expr x);
void print_glyph(Port* ios, Expr x);

// Type objects
Type BoolType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_BOOL,
  .obsize   = 0,
  .print_fn = print_bool
};

Type GlyphType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_GLYPH,
  .obsize   = 0,
  .print_fn = print_glyph
};

Type NumType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NUM,
  .obsize   = 0,
  .print_fn = print_num
};

// number APIs
Num as_num_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &NumType, x);
  return as_num(x);
}

Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

// shortcut for tagging pointers and small integers safely
uintptr_t as_fix(Expr x) {
  return x & XVMSK;
}

Expr tag_fix(uintptr_t i) {
  return ( i & XVMSK) | FIX_T;
}

void* as_ptr(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_ptr(void* ptr) {
  return ((uintptr_t)ptr) | FIX_T;
}

void print_num(Port* ios, Expr x) {
  pprintf(ios, "%g", as_num(x));
}

// boolean APIs
Bool as_bool(Expr x) {
  return x == TRUE;
}

Expr tag_bool(Bool b) {
  return b ? TRUE : FALSE;
}

void print_bool(Port* ios, Expr x) {
  pprintf(ios, x == TRUE ? "true" : "false");
}

// glyph APIs
Glyph as_glyph(Expr x) {
  return x & XVMSK;
}

Glyph as_glyph_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &GlyphType, x);
  return as_glyph(x);
}

Expr tag_glyph(Glyph x) {
  return ((Expr)x) | GLYPH_T;
}

void print_glyph(Port* ios, Expr x) {
  Glyph g = as_glyph(x);

  if ( g < CHAR_MAX && CharNames[g] )
    pprintf(ios, "\\%s", CharNames[g]);

  else
    pprintf(ios, "\\%c", g);
}
