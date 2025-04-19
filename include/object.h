#ifndef rl_object_h
#define rl_object_h

#include "common.h"

#include <stdio.h>

typedef enum {
    EXP_NUL=1,
    EXP_EOS,
    EXP_IOS,
    EXP_FUN,
    EXP_CHUNK,
    EXP_ENV,
    EXP_BIN,
    EXP_STR,
    EXP_SYM,
    EXP_LIST,
    EXP_VEC,
    EXP_MAP,
    EXP_GLYPH,
    EXP_NUM
} ExpType;

typedef enum {
  OKAY,
  PROBLEM
} Status;

typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double    Num;
typedef char      Glyph;
typedef struct    Obj   Obj;
typedef struct    IOS   IOS;
typedef struct    Fun   Fun;
typedef struct    Chunk Chunk;
typedef struct    Env   Env;
typedef struct    Bin   Bin;
typedef struct    Str   Str;
typedef struct    Sym   Sym;
typedef struct    List  List;
typedef struct    Vec   Vec;
typedef struct    Map   Map;

typedef Status (*PrimFn)(size_t n, Expr* args, Expr* r);

typedef union {
  Expr expr;
  Num  num;
  Obj* obj;
  Nul  nul;
} Val;

#define HEAD                                    \
  Obj* heap;                                    \
  ExpType type;                                 \
  bool marked

struct Obj {
  HEAD;
};

struct IOS {
  HEAD;

  FILE* val;
};

struct Fun {
  HEAD;

  Chunk* code;
  Env*   env;
};

struct Chunk {
  HEAD;

  Map* vars;
  Vec* vals;
  Bin* instr;
};

struct Env {
  HEAD;

  bool bound;
  bool open;

  Env* parent;
  Map* mapping;

  union {
    Expr* space;
    Vec*  vals;
  };
};

struct Bin {
  HEAD;

  void* val;
  size_t count, max_count;
};

struct Str {
  HEAD;

  char*     val;
  uintptr_t hash;
};

struct Sym {
  HEAD;

  Str*      val;
  uintptr_t idno;
  uintptr_t hash;
};

struct List {
  HEAD;

  Expr   head;
  List*  tail;
  size_t count;
};

struct Vec {
  HEAD;

  Expr* vals;
  size_t count, max_count;
};

typedef struct {
  Expr key;
  Expr val;
} Bucket;

struct Map {
  HEAD;

  Bucket* kvs;
  size_t count, max_count, ts_count;
};

#endif
