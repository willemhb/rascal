#ifndef rl_val_h
#define rl_val_h

#include <stdio.h>

#include "common.h"
#include "util/collection.h"
#include "opcode.h"

// common object header
#define HEAD                                        \
  Obj* heap;                                        \
  Type* type;                                       \
  union {                                           \
    struct {                                        \
      uintptr_t bfields  : 16;                      \
      uintptr_t hashcode : 48;                      \
    };                                              \
    struct {                                        \
      uintptr_t black    :  1;                       \
      uintptr_t gray     :  1;                       \
      uintptr_t nosweep  :  1;                       \
      uintptr_t flags    : 13;                       \
      uintptr_t _pad     : 48;                       \
    };                                               \
  }

// Types
// builtin type codes
enum {
  EXP_NONE=1,
  EXP_NUL,
  EXP_EOS,
  EXP_BOOL,
  EXP_GLYPH,
  EXP_TYPE,
  EXP_CHUNK,
  EXP_ALIST,
  EXP_BUF16,
  EXP_REF,
  EXP_UPV,
  EXP_ENV,
  EXP_PORT,
  EXP_FUN,
  EXP_METHOD,
  EXP_MTABLE,
  EXP_SYM,
  EXP_STR,
  EXP_LIST,
  EXP_NUM
};

typedef enum {
  // memory flags
  FL_BLACK    = 0x8000,
  FL_GRAY     = 0x4000,
  FL_NOSWEEP  = 0x2000,
} ExpFlags;

struct Obj {
  HEAD;
};

// Type object - first class type representation
struct Type {
  HEAD;

  int tag; // for easy type comparison and hashing
  Sym* name;
  size_t obsize;
  HasFn has_fn;
  PrintFn print_fn;
  HashFn hash_fn;
  EgalFn egal_fn;
  CloneFn clone_fn;
  TraceFn trace_fn;
  FreeFn free_fn;
};

// Expression tags
#define QNAN   0x7ffc000000000000ul

#define XTMSK  0xffff000000000000ul
#define XVMSK  0x0000fffffffffffful

#define NONE_T  0x7ffc000000000000ul
#define NUL_T   0x7ffd000000000000ul
#define EOS_T   0x7ffe000000000000ul
#define BOOL_T  0x7fff000000000000ul
#define GLYPH_T 0xfffc000000000000ul
#define FIX_T   0xfffe000000000000ul
#define OBJ_T   0xffff000000000000ul

// special values
#define NONE    0x7ffc000000000000ul
#define NUL     0x7ffd000000000000ul
#define EOS     0x7ffe0000fffffffful
#define TRUE    0x7fff000000000001ul
#define FALSE   0x7fff000000000000ul
#define RL_ZERO 0x0000000000000000ul
#define RL_ONE  0x3ff0000000000000ul

// convenience macros
#define exp_tag(x)     ((x) & XTMSK)
#define exp_val(x)     ((x) & XVMSK)
#define head(x)        ((Obj*)as_obj(x))
#define as_type(x)     ((Type*)as_obj(x))

// Array and table types are now declared in collection.h
void trace_exprs(RlState* rls, Exprs* xs);
void trace_objs(RlState* rls, Objs* os);

// type API
void init_builtin_type(RlState* rls, Type* type, char* name);
void register_builtin_types(RlState* rls);
Type* type_of(Expr x);
bool has_type(Expr x, Type* t);
char* type_name(Type* t);

// expression APIs
hash_t hash_exp(Expr x);
bool egal_exps(Expr x, Expr y);
void mark_exp(RlState* rls, Expr x);

// object API
void* as_obj(Expr x);
void* as_obj_s(RlState* s, char* fn, Type* t, Expr x);
Expr  tag_obj(void* ptr);
void* mk_obj(RlState* rls, Type* type, flags_t flags);
void* mk_obj_s(RlState* rls, Type* type, flags_t flags);
void* clone_obj(RlState* rls, void* ptr);
void* clone_obj_s(RlState* rls, void* ptr);
void  mark_obj(RlState* rls, void* ptr);
void  unmark_obj(void* ptr);
void  free_obj(RlState* rls, void *ptr);

// Type extern declarations
extern Type TypeType, NoneType, NulType, EosType, BoolType, GlyphType,
  ChunkType, AlistType, Buf16Type, RefType, UpValType, EnvType,
  PortType, FunType, MethodType, MethodTableType, SymType, StrType,
  ListType, NumType;

// safe cast macro
#define as_type_s(rls, f, x) ((Type*)as_obj_s(rls, f, &TypeType, x))

// type predicates
#define is_glyph(x)       has_type(x, &GlyphType)
#define is_num(x)         has_type(x, &NumType)
#define is_sym(x)         has_type(x, &SymType)
#define is_method(x)      has_type(x, &MethodType)
#define is_fun(x)         has_type(x, &FunType)
#define is_mtable(x)      has_type(x, &MethodTableType)
#define is_list(x)        has_type(x, &ListType)

#endif
