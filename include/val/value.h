#ifndef rl_val_value_h
#define rl_val_value_h

#include "labels.h"
#include "values.h"
#include "runtime.h"

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

#define UNIT       0xffff000100000000ul
#define BOOL       0xffff000200000000ul
#define GLYPH      0xffff000300000000ul
#define SMALL      0xffff000400000000ul

// common singletons
#define NUL        0xffff000100000000ul // UNIT  |  0
#define NOTHING    0xffff000100000001ul // UNIT  |  1 (sentinel, not a proper value, shouldn't leak)
#define TRUE       0xffff000200000001ul // BOOL  |  1
#define FALSE      0xffff000200000000ul // BOOL  |  0
#define EOS        0xffff0003fffffffful // GLYPH | -1

// Rascal true and false representation

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
#define vtype_of(x)     generic2(vtype_of, x, x)
#define type_of(x)      generic2(type_of, x, x)
#define has_type(x, t)  generic2(has_type, x, x, t)
#define has_vtype(x, t) generic2(has_vtype, x, x, t)

#define vtbl(x)                                 \
  generic((x),                                  \
          Val:val_vtbl,                         \
          Type*:type_vtbl,                      \
          VType:vtype_vtbl,                     \
          default:obj_vtbl)(x)

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
          word_t:tag_num,                        \
          Ptr:tag_ptr,                           \
          sint16*:tag_ptr,                       \
          uint16*:tag_ptr,                       \
          char*:tag_ptr,                         \
          Val*:tag_ptr,                          \
          Obj*:tag_obj,                          \
          Box*:tag_obj,                          \
          Func*:tag_obj,                         \
          Chunk*:tag_obj,                        \
          Prim*:tag_obj,                         \
          MTRoot*:tag_obj,                       \
          MTNode*:tag_obj,                       \
          MTLeaf*:tag_obj,                       \
          Sym*:tag_obj,                          \
          Type*:tag_obj,                         \
          Port*:tag_obj,                         \
          Str*:tag_obj,                          \
          Bin*:tag_obj,                          \
          List*:tag_obj,                         \
          Vec*:tag_obj,                          \
          Map*:tag_obj,                          \
          MStr*:tag_obj,                         \
          MBin*:tag_obj,                         \
          MVec*:tag_obj,                         \
          MMap*:tag_obj,                         \
          VNode*:tag_obj,                        \
          VLeaf*:tag_obj,                        \
          MNode*:tag_obj,                        \
          MLeaf*:tag_obj,                        \
          MLeafs*:tag_obj,                       \
          Rt*:tag_obj,                           \
          Cntl*:tag_obj,                         \
          Ns*:tag_obj,                           \
          Env*:tag_obj,                          \
          Ref*:tag_obj,                          \
          Upv*:tag_obj )(x)

// tagging methods
Val tag_nul(Nul n);
Val tag_bool(Bool b);
Val tag_glyph(Glyph g);
Val tag_num(Num n);
Val tag_small(Small s);
Val tag_ptr(Ptr p);
Val tag_obj(void* p);

// type_of methods
Type* val_type_of(Val v);
Type* obj_type_of(void* x);

// has_type methods
bool val_has_type(Val v, Type* t);
bool obj_has_type(void* x, Type* t);

// vtype_of methods
VType val_vtype_of(Val v);
VType obj_vtype_of(void* p);

// has_vtype methods
bool val_has_vtype(Val v, VType t);
bool obj_has_vtype(void* p, VType t);

// VTable accessor methods
VTable* val_vtbl(Val v);
VTable* vtype_vtbl(VType t);
VTable* type_vtbl(Type* t);
VTable* obj_vtbl(void* p);

#endif
