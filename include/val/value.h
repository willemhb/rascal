#ifndef rl_val_value_h
#define rl_val_value_h

#include "error.h"
#include "labels.h"

/* tags and masks */
#define QNAN       0x7ff8000000000000UL
#define SIGN       0x8000000000000000UL

#define TAG_BITS   0xffff000000000000UL
#define DATA_BITS  0x0000ffffffffffffUL

// value tags
#define REAL    0x0000000000000000UL // dummy tag
#define NUL     0x7ffc000000000000UL
#define BOOL    0x7ffd000000000000UL
#define LABL    0x7ffe000000000000UL
#define PTR     0x7fff000000000000UL
#define GLYPH   0xfffc000000000000UL
#define OBJECT  0xfffd000000000000UL

// sentinel tags
#define NOTHING 0xfffe000000000000UL
#define OFFSET  0xffff000000000000UL

#define TRUE    0x7ffd000000000001UL // BOOL  | 1
#define FALSE   0x7ffd000000000000UL // BOOL  | 0

// sentinel values, shouldn't be visible in user code

/* Globals */
/* APIs */
// tagging/untagging macros & functions
static inline Val tag_bits(Val x) {
  return x & TAG_BITS;
}

static inline Val data_bits(Val x) {
  return x & DATA_BITS;
}

#define as_obj(x)                               \
  generic((x),                                  \
          Val:val_as_obj,                       \
          default:ptr_as_obj)(x)

#define type_of(x)                              \
  generic((x),                                  \
          Val:type_of_val,                      \
          default:type_of_obj)(x)

#define size_of(x, o)                           \
  generic((x),                                  \
          Val:size_of_val,                      \
          default:size_of_obj)(x, o)

#define has_type(x, t)                          \
  generic((x),                                  \
          Val:val_has_type,                     \
          default:obj_has_type)(x, t)

// lower-level tag macro
#define tagv(v, t) (((Val)(v)) | (t))

// big ass tag macro
#define tag(x)                                  \
  generic((x),                                  \
          Nul:tag_nul,                          \
          Bool:tag_bool,                        \
          Label:tag_lbl,                        \
          Ptr:tag_ptr,                          \
          short*:tag_ptr,                       \
          ushort*:tag_ptr,                      \
          char*:tag_ptr,                        \
          Val*:tag_ptr,                         \
          Glyph:tag_glyph,                      \
          char:tag_glyph,                       \
          Real:tag_real,                        \
          Obj*:tag_obj,                         \
          Port*:tag_obj,                        \
          Func*:tag_obj,                        \
          Sym*:tag_obj,                         \
          Str*:tag_obj,                         \
          Bin*:tag_obj,                         \
          Pair*:tag_obj,                        \
          List*:tag_obj,                        \
          Vec*:tag_obj,                         \
          Map*:tag_obj,                         \
          Buffer*:tag_obj,                      \
          Alist*:tag_obj,                       \
          Table*:tag_obj,                       \
          UpVal*:tag_obj,                       \
          VNode*:tag_obj,                       \
          MNode*:tag_obj,                       \
          State*:tag_obj)(x)

#define untag(d, x)                             \
  ((typeof(d))(generic((d),                     \
                       Obj*:val_as_obj,         \
                       Nul:as_nul,              \
                       Bool:as_bool,            \
                       Ptr:as_ptr,              \
                       short*:as_ptr,           \
                       ushort*:as_ptr,          \
                       Val*:as_ptr,             \
                       FunPtr:as_fptr,          \
                       Type*:val_as_obj,        \
                       Func*:val_as_obj,        \
                       Proto*:val_as_obj,       \
                       PrimFn*:val_as_obj,      \
                       GenFn*:val_as_obj,       \
                       MT*:val_as_obj,          \
                       Cntl*:val_as_obj,        \
                       Sym*:val_as_obj,         \
                       Env*:val_as_obj,         \
                       Ref*:val_as_obj,         \
                       UpVal*:val_as_obj,       \
                       Glyph:val_as_obj,        \
                       Port*:val_as_obj,        \
                       Str*:val_as_obj,         \
                       Bin*:val_as_obj,         \
                       MStr*:val_as_obj,        \
                       MBin*:val_as_obj,        \
                       RT*:val_as_obj,          \
                       Arity:as_arity,          \
                       Label:as_small,          \
                       ushort:as_small,         \
                       Small:as_small,          \
                       Real:as_real,            \
                       Pair*:val_as_obj,        \
                       List*:val_as_obj,        \
                       MPair*:val_as_obj,       \
                       MList*:val_as_obj,       \
                       Vec*:val_as_obj,         \
                       VNode*:val_as_obj,       \
                       MVec*:val_as_obj,        \
                       Alist*:val_as_obj,       \
                       Map*:val_as_obj,         \
                       MNode*:val_as_obj,       \
                       MMap*:val_as_obj,        \
                       SCache*:val_as_obj,      \
                       EMap*:val_as_obj,        \
                       NSMap*:val_as_obj)(x)))

// tagging methods
Val tag_nul(Nul n);
Val tag_bool(Bool b);
Val tag_lbl(Label l);
Val tag_glyph(Glyph g);
Val tag_real(Real n);
Val tag_ptr(Ptr p);
Val tag_obj(void* p);

// casting methods
Nul    as_nul(Val x);
Bool   as_bool(Val x);
Label  as_lbl(Val x);
Glyph  as_glyph(Val x);
Real   as_real(Val x);
Ptr    as_ptr(Val x);
Obj*   val_as_obj(Val v);
Obj*   ptr_as_obj(void* p);

// type_of methods
Type* type_of_val(Val v);
Type* type_of_obj(void* p);

// has_type methods
bool val_has_type(Val v, Type t);
bool obj_has_type(void* p, Type t);

// size_of methods
size_t size_of_val(Val x, bool o);
size_t size_of_obj(void* x, bool o);

// value predicates
bool  is_nul(Val x);
bool  is_bool(Val x);
bool  is_glyph(Val x);
bool  is_ptr(Val x);
bool  is_real(Val x);
bool  is_obj(Val x);

#endif
