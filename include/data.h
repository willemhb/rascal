#ifndef rl_data_h
#define rl_data_h

#include <stdio.h>

#include "common.h"
#include "collection.h"

// Types
// Expression type codes
typedef enum {
  EXP_NONE,
  EXP_NUL,
  EXP_EOS,
  EXP_ENV,
  EXP_FUN,
  EXP_SYM,
  EXP_STR,
  EXP_LIST,
  EXP_NUM
} ExpType;

// Expression types
typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double    Num;
typedef struct    Obj  Obj;
typedef struct    Env  Env;
typedef struct    Fun  Fun;
typedef struct    Str  Str;
typedef struct    Sym  Sym;
typedef struct    List List;

typedef union {
  Expr expr;
  Num  num;
  Obj* obj;
  Nul  nul;
} Val;

// Internal types
typedef void   (*PrintFn)(FILE* ios, Expr x);
typedef hash_t (*HashFn)(Expr x);
typedef bool   (*EgalFn)(Expr x, Expr y);
typedef void   (*TraceFn)(void* ob);
typedef void   (*FreeFn)(void* ob);

typedef struct {
  ExpType type;
  char*   name;
  size_t  obsize;
  PrintFn print_fn;
  HashFn  hash_fn;
  EgalFn  egal_fn;
  TraceFn trace_fn;
  FreeFn  free_fn;
} ExpTypeInfo;

extern ExpTypeInfo Types[];

#define HEAD                                     \
  Obj* heap;                                     \
  ExpType type;                                  \
  union {                                        \
    flags_t bfields;                             \
    struct {                                     \
      flags_t black   :   1;                      \
      flags_t gray    :   1;                      \
      flags_t nosweep :   1;                      \
      flags_t flags    : 29;                     \
    };                                           \
  }

typedef enum {
  FL_BLACK   = 0x80000000,
  FL_GRAY    = 0x40000000,
  FL_NOSWEEP = 0x20000000
} ExpFlags;

typedef enum {
  OP_NOOP,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
} OpCode;

struct Obj {
  HEAD;
};

// Table types
TABLE_API(Strings, char*, Str*, strings);
TABLE_API(EMap, Sym*, int, emap);

struct Env {
  HEAD;

  EMap  map;
  Alist vals;
};

struct Fun {
  HEAD;

  Sym*   name;
  OpCode label;
};

struct Sym {
  HEAD;

  Str*   val;
  hash_t hash;
};

struct Str {
  HEAD;

  char*  val;
  size_t count;
  hash_t hash;
};

struct List {
  HEAD;

  Expr   head;
  List*  tail;
  size_t count;
};

// Expression tags
#define QNAN   0x7ffc000000000000ul

#define XTMSK  0xffff000000000000ul
#define XVMSK  0x0000fffffffffffful

#define OBJ    0xfffc000000000000ul
#define NUL    0xffff000000000000ul
#define NONE   0x7ffe000000000000ul
#define EOS_T  0x7ffd000000000000ul
#define EOS    0x7ffd0000fffffffful

// forward declarations
// expression APIs
ExpType exp_type(Expr x);
bool    has_type(Expr x, ExpType t);
ExpTypeInfo* exp_info(Expr x);
hash_t hash_exp(Expr x);
bool   egal_exps(Expr x, Expr y);
void   mark_exp(Expr x);

// object API
void* as_obj(Expr x);
Expr  tag_obj(void* obj);
void* mk_obj(ExpType type, flags_t flags);
void  mark_obj(void* ptr);
void  free_obj(void *ptr);

// function API
Fun* mk_fun(Sym* name, OpCode op);
void def_builtin_fun(char* name, OpCode op);

// environment API
Env* mk_env(void);
Expr env_get(Env* e, Sym* n);
int  env_def(Env* e, Sym* n);
void env_set(Env* e, Sym* n, Expr x);
void def_builtin(Env* e, Sym* n, Expr x);

// symbol API
Sym* mk_sym(char* cs);
bool sym_val_eql(Sym* s, char* v);

// string API
Str* mk_str(char* cs);

// list API
List*  empty_list(void);
List*  mk_list(size_t n, Expr* xs);
List*  cons(Expr hd, List* tl);
Expr   list_ref(List* xs, int n);

// number API
Num  as_num(Expr x);
Expr tag_num(Num n);

// convenience macros
#define exp_tag(x)     ((x) & XTMSK)
#define head(x)        ((Obj*)as_obj(x))
#define as_fun(x)      ((Fun*)as_obj(x))
#define as_env(x)      ((Env*)as_obj(x))
#define as_sym(x)      ((Sym*)as_obj(x))
#define as_str(x)      ((Str*)as_obj(x))
#define as_list(x)     ((List*)as_obj(x))

#define is_interned(s) ((s)->flags == true)
#define is_sym(x)      has_type(x, EXP_SYM)
#define is_fun(x)      has_type(x, EXP_FUN)
#define is_list(x)     has_type(x, EXP_LIST)

#endif
