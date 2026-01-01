#include "val/primitive.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"

// globals
// forward declarations
void box_num(Box* box, Expr x);
void box_bool(Box* box, Expr x);
void box_glyph(Box* box, Expr x);

Expr unbox_num(Box* box);
Expr unbox_bool(Box* box);
Expr unbox_glyph(Box* box);

bool init_num(Box* box, void* spc);
bool init_bool(Box* box, void* spc);
bool init_glyph(Box* box, void* spc);

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
  .print_fn = print_bool,
  .box_fn   = box_bool,
  .unbox_fn = unbox_bool,
};

Type GlyphType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_GLYPH,
  .obsize   = 0,
  .print_fn = print_glyph,
  .box_fn   = box_glyph,
  .unbox_fn = unbox_glyph,
};

Type NumType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NUM,
  .obsize   = 0,
  .print_fn = print_num,
  .box_fn   = box_num,
  .unbox_fn = unbox_num,
};

// boxing APIs
Box* box_expr(RlState* rls, Expr x) {
  Type* type = type_of(x);
  Box* out = NULL;

  if ( type->box_fn )
    out = mk_box(rls, type, 0, x);

  return out;
}

Expr unbox_box(Box* box) {
  Type* type = box->type;

  return type->unbox_fn(box);
}

Box* mk_box(RlState* rls, Type* type, int flags, Expr init) {
  Box* out = allocate(rls, sizeof(Box));

  out->type    = type;
  out->bfields = FL_GRAY | flags;

  assert(type->box_fn != NULL);
  type->box_fn(out, init);
  add_to_managed(rls, out);

  return out;
}

Box* mk_box_s(RlState* rls, Type* type, int flags, Expr init) {
  Box* out = mk_box(rls, type, flags, init);
  stack_push(rls, tag_box(out));
  return out;
}

// number APIs
Num as_num_s(RlState* rls, Expr x) {
  require_argtype(rls, &NumType, x);
  return as_num(x);
}

Num as_num(Expr x) {
  if ( (x & XTMSK) == FIX_T )
    return as_fix(x);

  union { Expr expr; Num num; } bits = { .expr = x };

  return bits.num;
}

Expr tag_num(Num n) {
  union { Expr expr; Num num; } bits = { .num = n };

  return bits.expr;
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
  if ( expr_tag(x) == FIX_T )
    pprintf(ios, "0x%.12x", as_fix(x));

  else
    pprintf(ios, "%g", as_num(x));
}

void box_num(Box* box, Expr init) {
  box->fixnum = init;

  if ( (init & XTMSK) == FIX_T )
    box->flags = true;
}

Expr unbox_num(Box* box) {
  return box->fixnum;
}

bool init_num(Box* box, void* spc) {
  if ( box->flags )
    *(uptr_t*)spc = box->fixnum;

  else
    *(double*)spc = box->num;

  return true;
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

void box_bool(Box* box, Expr init) {
  box->boolean = init == TRUE;
}

Expr unbox_bool(Box* box) {
  return box->boolean ? TRUE : FALSE;
}

bool init_bool(Box* box, void* spc) {
  *(Bool*)spc = box->boolean;

  return true;
}

// glyph APIs
Glyph as_glyph(Expr x) {
  return x & WVMSK;
}

Glyph as_glyph_s(RlState* rls, Expr x) {
  require_argtype(rls, &GlyphType, x);
  return as_glyph(x);
}

Expr tag_glyph(Glyph x) {
  return ((Expr)x) | GLYPH_T;
}

void box_glyph(Box* box, Expr init) {
  box->glyph = as_glyph(init);
}

Expr unbox_glyph(Box* box) {
  return tag_glyph(box->glyph);
}

bool init_glyph(Box* box, void* spc) {
  *(Glyph*)spc = box->glyph;

  return true;
}

char* char_name(Glyph g) {
  switch ( g ) {
    case 0xffffffffu:  return "eos";
    case '\0':         return "nul";
    case '\n':         return "newline";
    case ' ':          return "space";
    case '\a':         return "bel";
    case '\t':         return "tab";
    case '\r':         return "return";
    case '\f':         return "formfeed";
    case '\v':         return "vtab";
    case '\b':         return "backspace";
    default:           return NULL;
  }
}

void print_glyph(Port* ios, Expr x) {
  Glyph g = as_glyph(x);
  char* n = char_name(g);

  if ( n )
    pprintf(ios, "\\%s", n);

  else
    pprintf(ios, "\\%c", g);
}
