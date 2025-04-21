#ifndef rl_data_h
#define rl_data_h

#include <stdio.h>

#include "common.h"
#include "collection.h"
#include "opcode.h"

// Types
// Expression type codes
typedef enum {
  EXP_NONE,
  EXP_NUL,
  EXP_EOS,
  EXP_BOOL,
  EXP_CHUNK,
  EXP_ALIST,
  EXP_BUF16,
  EXP_REF,
  EXP_UPV,
  EXP_ENV,
  EXP_FUN,
  EXP_SYM,
  EXP_STR,
  EXP_LIST,
  EXP_NUM
} ExpType;

#define NUM_TYPES (EXP_NUM+1)

// Expression types
typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double    Num;
typedef bool      Bool;
typedef struct    Obj    Obj;
typedef struct    Chunk  Chunk;
typedef struct    Alist  Alist;
typedef struct    Buf16  Buf16;
typedef struct    Env    Env;
typedef struct    Fun    Fun;
typedef struct    Str    Str;
typedef struct    Sym    Sym;
typedef struct    List   List;

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
  Sym*    repr;       // rascal representation of the type (for now just a keyword)
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
      flags_t black   :   1;                     \
      flags_t gray    :   1;                     \
      flags_t nosweep :   1;                     \
      flags_t flags    : 29;                     \
    };                                           \
  }

typedef enum {
  FL_BLACK   = 0x80000000,
  FL_GRAY    = 0x40000000,
  FL_NOSWEEP = 0x20000000
} ExpFlags;

struct Obj {
  HEAD;
};

// Array types
ALIST_API(Exprs, Expr, exprs);
ALIST_API(Objs, void*, objs);
ALIST_API(Bin16, ushort_t, bin16);

void trace_exprs(Exprs* xs);
void trace_objs(Objs* os);

// Table types
TABLE_API(Strings, char*, Str*, strings);
TABLE_API(EMap, Sym*, int, emap);

struct Chunk {
  HEAD;

  Env*    vars;
  Alist*  vals;
  Buf16*  code;
};

// wrapper around a Stack object
struct Alist {
  HEAD;

  Exprs exprs;
};

// wrapper around a binary object
struct Buf16 {
  HEAD;

  Bin16 binary;
};

struct Env {
  HEAD;
  bool  local;
  int   arity;
  EMap  map;
  Exprs vals;
};

struct Fun {
  HEAD;

  Sym*   name;
  OpCode label;
  Chunk* chunk;
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

#define NONE_T  0x7ffd000000000000ul
#define NUL_T   0x7ffe000000000000ul
#define EOS_T   0x7fff000000000000ul
#define BOOL_T  0xfffc000000000000ul
#define GLYPH_T 0xfffd000000000000ul
#define FIX_T   0xfffe000000000000ul
#define OBJ_T   0xffff000000000000ul

// special values
#define NONE   0x7ffd000000000000ul
#define NUL    0x7ffe000000000000ul
#define EOS    0x7fff0000fffffffful
#define TRUE   0xfffc000000000001ul
#define FALSE  0xfffc000000000000ul

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

// chunk API
Chunk* mk_chunk(Env* vars, Alist* vals, Buf16* code);
void   dis_chunk(Chunk* chunk);

// alist API
Alist* mk_alist(void);
void   free_alist(void* ptr);
int    alist_push(Alist* a, Expr x);
Expr   alist_pop(Alist* a);
Expr   alist_get(Alist* a, int n);

// buf16 API
Buf16*  mk_buf16(void);
void    free_buf16(void* ptr);
int     buf16_write(Buf16* b, ushort_t *xs, int n);

// function API
Fun* as_fun_s(char* f, Expr x);
Fun* mk_fun(Sym* name, OpCode op, Chunk* code);
Fun* mk_builtin_fun(Sym* name, OpCode op);
Fun* mk_user_fun(Chunk* code);
void def_builtin_fun(char* name, OpCode op);
void disassemble(Fun* fun);

// environment API
Env* mk_env(bool local);
int  env_resolve(Env* e, Sym* n);
Expr env_get(Env* e, Sym* n);
Expr env_ref(Env* e, int n);
int  env_put(Env* e, Sym* n);
void env_set(Env* e, Sym* n, Expr x);
void env_refset(Env* e, int n, Expr x);

// symbol API
Sym* as_sym_s(char* f, Expr x);
Sym* mk_sym(char* cs);
bool sym_val_eql(Sym* s, char* v);

// string API
Str* mk_str(char* cs);

// list API
List*  as_list_s(char* f, Expr x);
List*  empty_list(void);
List*  mk_list(size_t n, Expr* xs);
List*  cons(Expr hd, List* tl);
Expr   list_ref(List* xs, int n);

// number API
Num       as_num_s(char* f, Expr x);
Num       as_num(Expr x);
Expr      tag_num(Num n);
uintptr_t as_fix(Expr x);
Expr      tag_fix(uintptr_t i);
void*     as_ptr(Expr x);
Expr      tag_ptr(void* ptr);

// boolean API
Bool as_bool(Expr x);
Expr tag_bool(Bool b);

// convenience macros
#define exp_tag(x)     ((x) & XTMSK)
#define head(x)        ((Obj*)as_obj(x))
#define as_fun(x)      ((Fun*)as_obj(x))
#define as_env(x)      ((Env*)as_obj(x))
#define as_sym(x)      ((Sym*)as_obj(x))
#define as_str(x)      ((Str*)as_obj(x))
#define as_list(x)     ((List*)as_obj(x))

#define is_interned(s)    ((s)->flags == true)
#define is_keyword(s)     (*(s)->val->val == ':')
#define is_local_env(e)   ((e)->local == true)
#define is_user_fn(f)     ((f)->label == OP_NOOP)
#define is_toplevel_fn(f) (!(f)->chunk->vars->local)
#define user_fn_argc(f)   ((f)->chunk->vars->arity)
#define is_sym(x)         has_type(x, EXP_SYM)
#define is_fun(x)         has_type(x, EXP_FUN)
#define is_list(x)        has_type(x, EXP_LIST)

#endif
