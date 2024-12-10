#ifndef rl_val_box_h
#define rl_val_box_h

#include "val/object.h"

/* Object for representing boxed immediates. Actual VType and Type will correspond to boxed value. */
/* C types */
struct Box {
  HEADER;

  union {
    Nul   nul;
    Bool  boolean;
    Glyph glyph;
    Num   num;
    Ptr   ptr;
  };
};

/* Globals */
// Type objects
extern Type NulType, BoolType, GlyphType, NumType, PtrType;

/* APIs */
// box/unbox
Box*   rl_box(Val x);
size64 rl_unbox(Val x, void* spc, size64 n);

// Nul API
#define is_nul(x) has_vtype(x, T_UNIT)
#define as_nul(x) generic2(as_nul, x, x)

Nul val_as_nul(Val x);
Nul obj_as_nul(void* x);

// Bool API
#define is_bool(x) has_vtype(x, T_BOOL)
#define as_bool(x) generic2(as_bool, x, x)

Bool val_as_bool(Val x);
Bool obj_as_bool(void* x);
bool is_truthy(Val x);

// Glyph API
#define is_glyph(x) has_vtype(x, T_GLYPH)
#define as_glyph(x) generic2(as_glyph, x, x)

Glyph val_as_glyph(Val x);
Glyph obj_as_glyph(void* x);

// Num API
#define is_num(x) has_vtype(x, T_NUM)
#define as_num(x) generic2(as_num, x, x)

Num val_as_num(Val x);
Num obj_as_num(void* x);

// Ptr API
#define is_ptr(x) has_vtype(x, T_PTR)
#define as_ptr(x) generic2(as_ptr, x, x)

Ptr val_as_ptr(Val x);
Ptr obj_as_ptr(void* x);

/* Initialization */
void rl_val_box_init(void);

#endif
