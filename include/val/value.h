#ifndef rl_value_h
#define rl_value_h

#include "status.h"

/* Initial declarations for Rascal value types and basic APIs */
// general types
typedef word_t        Value;  // tagged word. May be immediate or an object
typedef struct Object Object; // Compound or large values are stored in a boxed object

// miscellaneous immediate types
typedef nullptr_t Nul;
typedef bool      Boolean;
typedef void*     Pointer;
typedef funcptr_t FuncPtr;

// metaobject types
typedef struct Type Type;

// function types
typedef struct Closure Closure;
typedef struct Native  Native;
typedef struct Generic Generic;
typedef struct MTRoot  MTRoot;
typedef struct MTNode  MTNode;
typedef struct MTLeaf  MTLeaf;
typedef struct Control Control;

// identifier and environment types
typedef struct Symbol  Symbol;
typedef struct Environ Environ;
typedef struct Binding Binding;
typedef struct UpValue UpValue;

// text, binary, and IO types
typedef char             Glyph;
typedef struct Port      Port;
typedef struct String    String;
typedef struct Binary    Binary;
typedef struct MutStr    MutStr;
typedef struct MutBin    MutBin;
typedef struct ReadTable ReadTable;

// numeric types
typedef word_t        Arity; // 48-bit unsigned integer (can hold any valid hash or size)
typedef int           Small;
typedef double        Real;
typedef struct Big    Big;
typedef struct Ratio  Ratio;

// list and pair types
typedef struct Pair    Pair;
typedef struct List    List;
typedef struct MutPair MutPair;
typedef struct MutList MutList;

// array types
typedef struct Vector  Vector;
typedef struct VecNode VecNode;
typedef struct MutVec  MutVec;
typedef struct Alist   Alist;

// table types
typedef struct Map      Map;
typedef struct MapNode  MapNode;
typedef struct Map      Record;
typedef struct Map      Set;
typedef struct MutMap   MutMap;
typedef struct MutMap   MutSet;
typedef struct StrCache StrCache;
typedef struct EnvMap   EnvMap;

// internal function pointer types
typedef rl_status_t (*rl_trace_fn_t)(Object* obj);
typedef rl_status_t (*rl_destruct_fn_t)(Object* obj);
typedef rl_status_t (*rl_reader_fn_t)(Port* stream, Value* buffer);
typedef hash_t      (*rl_hash_fn_t)(Value x);
typedef bool        (*rl_egal_fn_t)(Value x, Value y);
typedef int         (*rl_order_fn_t)(Value x, Value y);
typedef size_t      (*rl_sizeof_fn_t)(Value x);
typedef rl_status_t (*rl_native_fn_t)(size_t argc, Value* args, Value* buffer);

/* tags and masks */
#define QNAN       0x7ff8000000000000UL
#define SIGN       0x8000000000000000UL

#define TAG_BITS   0xffff000000000000UL
#define WTAG_BITS  0xffffffff00000000UL
#define WDATA_BITS 0x00000000ffffffffUL
#define DATA_BITS  0x0000ffffffffffffUL

// wide tags
#define REAL      0x0000000000000000UL // dummy tag
#define ARITY     0x7ffc000000000000UL // arity value
#define CPTR      0x7ffd000000000000UL
#define FPTR      0x7ffe000000000000UL
#define OBJECT    0x7fff000000000000UL
#define LITTLE    0xffff000000000000UL // 32-bit value with wide tag

#define NUL       0xffff000000000000UL
#define BOOL      0xffff000100000000UL
#define GLYPH     0xffff000200000000UL
#define SMALL     0xffff000300000000UL
#define SENTINEL  0xffffffff00000000UL

#define TRUE      0x7ffd000000000001UL // BOOL | 1
#define FALSE     0x7ffd000000000000UL // BOOL | 0

// sentinel values, shouldn't be visible in user code
#define NOTHING   0xffffffff00000000UL // SENTINEL | 0

/* Globals */
// type objects
extern Type NulType, BooleanType, GlyphType, PointerType, FuncPtrType;

/* APIs */
// tagging/untagging macros & functions
static inline Value tag_of(Value x) {
  return (x & LITTLE) == LITTLE ? x & WTAG_BITS : x & TAG_BITS;
}

static inline Value untag(Value x) {
  return (x & LITTLE) == LITTLE ? x & WDATA_BITS : x & DATA_BITS;
}

// big ass tag macro
#define tag(x)                                  \
  generic((x),                                  \
          Object*:tag_obj,                      \
          Nul:tag_nul,                          \
          Boolean:tag_bool,                     \
          Pointer:tag_ptr,                      \
          FuncPtr:tag_fptr,                     \
          Type*:tag_obj,                        \
          Closure*:tag_obj,                     \
          Native*:tag_obj,                      \
          Generic*:tag_obj,                     \
          MTRoot*:tag_obj,                      \
          MTNode*:tag_obj,                      \
          MTLeaf*:tag_obj,                      \
          Control*:tag_obj,                     \
          Symbol*:tag_obj,                      \
          Environ*:tag_obj,                     \
          Binding*:tag_obj,                     \
          UpValue*:tag_obj,                     \
          Glyph:tag_glyph,                      \
          Port*:tag_obj,                        \
          String*:tag_obj,                      \
          Binary*:tag_obj,                      \
          MutStr*:tag_obj,                      \
          MutBin*:tag_obj,                      \
          ReadTable*:tag_obj,                   \
          Arity:tag_arity,                      \
          Small:tag_small,                      \
          Real:tag_real,                        \
          Big*:tag_obj,                         \
          Ratio*:tag_obj,                       \
          Pair*:tag_obj,                        \
          List*:tag_obj,                        \
          MutPair*:tag_obj,                     \
          MutList*:tag_obj,                     \
          Vector*:tag_obj,                      \
          VecNode*:tag_obj,                     \
          MutVec*:tag_obj,                      \
          Alist*:tag_obj,                       \
          Map*:tag_obj,                         \
          MapNode*:tag_obj,                     \
          MutMap*:tag_obj,                      \
          StrCache*:tag_obj,                    \
          EnvMap*:tag_obj                       \
          )(x)

#define as_obj(x)                               \
  generic((x),                                  \
          Value:val_as_obj,                     \
          default:ptr_as_obj)(x)

#define type_of(x)                              \
  generic((x),                                  \
          Value:type_of_val,                    \
          default:type_of_obj)(x)

#define size_of(x, o)                           \
  generic((x),                                  \
          Value:size_of_val,                    \
          default:size_of_obj)(x, o)

#define has_type(x, t)                          \
  generic((x),                                  \
          Value:val_has_type,                   \
          default:obj_has_type)(x, t)

// tagging methods
Value tag_nul(Nul n);
Value tag_bool(Boolean b);
Value tag_glyph(Glyph g);
Value tag_small(Small s);
Value tag_arity(Arity a);
Value tag_real(Real n);
Value tag_ptr(Pointer p);
Value tag_fptr(FuncPtr f);
Value tag_obj(void* p);

// casting methods
Nul     as_nul(Value x);
Boolean as_bool(Value x);
Glyph   as_glyph(Value x);
Small   as_small(Value x);
Arity   as_arity(Value x);
Real    as_real(Value x);
Pointer as_ptr(Value x);
FuncPtr as_fptr(Value x);
Object* val_as_obj(Value v);
Object* ptr_as_obj(void* p);

// type_of methods
Type* type_of_val(Value v);
Type* type_of_obj(void* p);

// has_type methods
bool  val_has_type(Value v, Type* t);
bool  obj_has_type(void* p, Type* t);

// size_of methods
size_t size_of_val(Value x, bool o);
size_t size_of_obj(void* x, bool o);

// value predicates
bool  is_nul(Value x);
bool  is_bool(Value x);
bool  is_glyph(Value x);
bool  is_cptr(Value x);
bool  is_fptr(Value x);
bool  is_obj(Value x);

#endif
