#ifndef rl_val_value_h
#define rl_val_value_h

#include "error.h"
#include "labels.h"

/* tags and masks */
#define QNAN       0x7ff8000000000000UL
#define SIGN       0x8000000000000000UL

#define TAG_BITS   0xffff000000000000UL
#define WTAG_BITS  0xffffffff00000000UL
#define WDATA_BITS 0x00000000ffffffffUL
#define DATA_BITS  0x0000ffffffffffffUL

// narrow tags
#define REAL      0x0000000000000000UL // dummy tag
#define ARITY     0x7ffc000000000000UL // arity value
#define CPTR      0x7ffd000000000000UL
#define FPTR      0x7ffe000000000000UL
#define OBJECT    0x7fff000000000000UL
#define LITTLE    0xffff000000000000UL // 32-bit value with wide tag

// wide tags
#define NUL       0xffff000000000000UL
#define BOOL      0xffff000100000000UL
#define GLYPH     0xffff000200000000UL
#define SMALL     0xffff000300000000UL
#define SENTINEL  0xffffffff00000000UL

#define TRUE      0xffff000100000001UL // BOOL  | 1
#define FALSE     0xffff000100000000UL // BOOL  | 0
#define ZERO      0xffff000300000000UL // SMALL | 0
#define ONE       0xffff000300000001UL // SMALL | 1

// sentinel values, shouldn't be visible in user code
#define NOTHING   0xffffffff00000000UL // SENTINEL | 0

/* Globals */
// type objects
extern Type NulType, BoolType, PtrType, FunPtrType;

/* APIs */
// tagging/untagging macros & functions
static inline Val tag_of(Val x) {
  return (x & LITTLE) == LITTLE ? x & WTAG_BITS : x & TAG_BITS;
}

static inline Val untag(Val x) {
  return (x & LITTLE) == LITTLE ? x & WDATA_BITS : x & DATA_BITS;
}

// big ass tag macro
#define tag(x)                                  \
  generic((x),                                  \
          Obj*:tag_obj,                         \
          Nul:tag_nul,                          \
          Bool:tag_bool,                        \
          Ptr:tag_ptr,                          \
          short*:tag_ptr,                       \
          ushort*:tag_ptr,                      \
          FunPtr:tag_fptr,                      \
          Type*:tag_obj,                        \
          Func*:tag_obj,                        \
          Closure*:tag_obj,                     \
          Native*:tag_obj,                      \
          Primitive*:tag_obj,                   \
          Generic*:tag_obj,                     \
          MTNode*:tag_obj,                      \
          Cntl*:tag_obj,                        \
          Sym*:tag_obj,                         \
          Env*:tag_obj,                         \
          Ref*:tag_obj,                         \
          UpVal*:tag_obj,                       \
          Glyph:tag_glyph,                      \
          Port*:tag_obj,                        \
          Str*:tag_obj,                         \
          Bin*:tag_obj,                         \
          MStr*:tag_obj,                        \
          MBin*:tag_obj,                        \
          RT*:tag_obj,                          \
          Arity:tag_arity,                      \
          Label:tag_small,                      \
          Error:tag_small,                      \
          Small:tag_small,                      \
          Real:tag_real,                        \
          Big*:tag_obj,                         \
          Ratio*:tag_obj,                       \
          Pair*:tag_obj,                        \
          List*:tag_obj,                        \
          MPair*:tag_obj,                       \
          MList*:tag_obj,                       \
          Vec*:tag_obj,                         \
          VNode*:tag_obj,                       \
          MVec*:tag_obj,                        \
          Alist*:tag_obj,                       \
          Map*:tag_obj,                         \
          MNode*:tag_obj,                       \
          MMap*:tag_obj,                        \
          SCache*:tag_obj,                      \
          EMap*:tag_obj                         \
          )(x)

#define untag(s, x)                                 \
  ((typeof(s))generic((s),                          \
                      Obj*:as_obj,                  \
                      Nul:as_nul,                   \
                      Bool:as_bool,                 \
                      Ptr:as_ptr,                   \
                      short*:as_ptr,                \
                      ushort*:as_ptr,               \
                      FunPtr:as_fptr,               \
                      Type*:as_obj,                 \
                      Func*:as_obj,                 \
                      Closure*:as_obj,              \
                      Native*:as_obj,               \
                      Primitive*:as_obj,            \
                      Generic*:as_obj,              \
                      MTNode*:as_obj,               \
                      Cntl*:as_obj,                 \
                      Sym*:as_obj,                  \
                      Env*:as_obj)(x))

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

// tagging methods
Val tag_nul(Nul n);
Val tag_bool(Bool b);
Val tag_glyph(Glyph g);
Val tag_small(Small s);
Val tag_arity(Arity a);
Val tag_real(Real n);
Val tag_ptr(Ptr p);
Val tag_fptr(FunPtr f);
Val tag_obj(void* p);

// casting methods
Nul    as_nul(Val x);
Bool   as_bool(Val x);
Glyph  as_glyph(Val x);
Small  as_small(Val x);
Arity  as_arity(Val x);
Real   as_real(Val x);
Ptr    as_ptr(Val x);
FunPtr as_fptr(Val x);
Obj*   val_as_obj(Val v);
Obj*   ptr_as_obj(void* p);

// type_of methods
Type* type_of_val(Val v);
Type* type_of_obj(void* p);

// has_type methods
bool val_has_type(Val v, Type* t);
bool obj_has_type(void* p, Type* t);

// size_of methods
size_t size_of_val(Val x, bool o);
size_t size_of_obj(void* x, bool o);

// value predicates
bool  is_nul(Val x);
bool  is_bool(Val x);
bool  is_glyph(Val x);
bool  is_cptr(Val x);
bool  is_fptr(Val x);
bool  is_obj(Val x);

#endif
