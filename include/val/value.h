#ifndef rl_val_value_h
#define rl_val_value_h

#include "labels.h"
#include "values.h"
#include "runtime.h"

#include "vm/type.h"

/* tags and masks */
#define QNAN       0x7ff8000000000000ul
#define SIGN       0x8000000000000000ul

#define TAG_BITS   0xffff000000000000ul
#define DATA_BITS  0x0000fffffffffffful
#define WTAG_BITS  0xffffffff00000000ul
#define WDATA_BITS 0x00000000fffffffful
#define TAG_SHIFT  48
#define WTAG_SHIFT 32
#define WTYPE_MASK 255

// value tags
#define REAL       0x0000000000000000ul
#define CPOINTER   0x7ffc000000000000ul
#define ARITY      0x7ffd000000000000ul
#define OBJECT     0x7ffe000000000000ul
#define LITTLE     0xffff000000000000ul

#define NUL        0xffff000100000000ul
#define BOOL       0xffff000200000000ul
#define GLYPH      0xffff000300000000ul
#define SMALL      0xffff000400000000ul

/* not a real value (shouldn't leak), but used to detect absent or uninitialized
   values where a value is expected. Eg, unused table nodes have both their key and
   value set to NOTHING, while tombstones have their key set to NOTHING but retain
   a valid value. */
#define NOTHING    0xffff001000000001ul // NUL   |  1

// Rascal true and false representation
#define TRUE       0xffff000200000001ul // BOOL  |  1
#define FALSE      0xffff000200000000ul // BOOL  |  0
#define EOS        0xffff0003fffffffful // GLYPH | -1

/* Globals */
/* APIs */
// tagging/untagging macros & functions
static inline Val tag_bits(Val x) {
  return x & TAG_BITS;
}

static inline Val data_bits(Val x) {
  return x & DATA_BITS;
}

static inline Val wtag_bits(Val x) {
  return x & WTAG_BITS;
}

static inline Val wdata_bits(Val x) {
  return x & WDATA_BITS;
}

// Value accessors and metaccessor shortcuts
#define as_obj(x)      generic2(as_obj, x, x)
#define type_of(x)     generic2(type_of, x, x)
#define has_type(x, t) generic2(has_type, x, x, t)

#define vtbl(x)                                 \
  generic((x),                                  \
          Val:val_vtbl,                         \
          Type:type_vtbl,                       \
          default:obj_vtbl)(x)

// VTable accessor macros
#define tname(x)       (vtbl(x)->name)
#define obsize(x)      (vtbl(x)->obsize)
#define tracefn(x)     (vtbl(x)->tracefn)
#define freefn(x)      (vtbl(x)->freefn)
#define hashfn(x)      (vtbl(x)->hashfn)
#define egalfn(x)      (vtbl(x)->egalfn)
#define orderfn(x)     (vtbl(x)->orderfn)

// lower-level tag macro
#define tagv(v, t) (((Val)(v)) | (t))

// big ass tag macro
#define tag(x)                                   \
  generic((x),                                   \
          Nul:tag_nul,                           \
          Bool:tag_bool,                         \
          Glyph:tag_glyph,                       \
          char:tag_glyph,                        \
          Small:tag_small,                       \
          Num:tag_num,                           \
          Ptr:tag_ptr,                           \
          sint16*:tag_ptr,                       \
          uint16*:tag_ptr,                       \
          char*:tag_ptr,                         \
          Val*:tag_ptr,                          \
          Obj*:tag_obj,                          \
          PrimFn*:tag_obj,                       \
          UserFn*:tag_obj,                       \
          Sym*:tag_obj,                          \
          Port*:tag_obj,                         \
          Str*:tag_obj,                          \
          List*:tag_obj,                         \
          Vec*:tag_obj,                          \
          Pair*:tag_obj,                         \
          Buffer*:tag_obj,                       \
          Alist*:tag_obj,                        \
          Table*:tag_obj,                        \
          Env*:tag_obj,                          \
          Ref*:tag_obj,                          \
          VNode*:tag_obj )(x)

// tagging methods
Val tag_nul(Nul n);
Val tag_bool(Bool b);
Val tag_glyph(Glyph g);
Val tag_num(Num n);
Val tag_small(Small s);
Val tag_ptr(Ptr p);
Val tag_obj(void* p);

// casting methods
Nul    as_nul(Val x);
Bool   as_bool(Val x);
Glyph  as_glyph(Val x);
Num    as_num(Val x);
Ptr    as_ptr(Val x);
Obj*   val_as_obj(Val v);
Obj*   obj_as_obj(void* p);

// type_of methods
Type val_type_of(Val v);
Type obj_type_of(void* p);

// has_type methods
bool val_has_type(Val v, Type t);
bool obj_has_type(void* p, Type t);

// VTable accessor methods
VTable* val_vtbl(Val v);
VTable* type_vtbl(Type t);
VTable* obj_vtbl(void* p);

// value predicates
bool  is_nul(Val x);
bool  is_bool(Val x);
bool  is_glyph(Val x);
bool  is_num(Val x);
bool  is_big(Val x);
bool  is_small(Val x);
bool  is_ptr(Val x);
bool  is_obj(Val x);

#endif
