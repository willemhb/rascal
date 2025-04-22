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
  EXP_GLYPH,
  EXP_CHUNK,
  EXP_ALIST,
  EXP_BUF16,
  EXP_REF,
  EXP_UPV,
  EXP_ENV,
  EXP_PORT,
  EXP_FUN,
  EXP_SYM,
  EXP_STR,
  EXP_LIST,
  EXP_NUM
} ExpType;

#define NUM_TYPES (EXP_NUM+1)

typedef struct {
  ExpType type;
  char*   name;
  Sym*    repr;       // rascal representation of the type (for now just a keyword)
  size_t  obsize;
  PrintFn print_fn;
  HashFn  hash_fn;
  EgalFn  egal_fn;
  CloneFn clone_fn;
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
TABLE_API(EMap, Sym*, Ref*, emap);

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

// variable reference metadata

typedef enum {
  REF_UNDEF,
  REF_GLOBAL,
  REF_LOCAL,
  REF_LOCAL_UPVAL,
  REF_CAPTURED_UPVAL
} RefType;

struct Ref {
  HEAD;

  Ref*    captures;
  Sym*    name;
  RefType ref_type;
  int     offset;
};

// special indirecting containers for closure variables
struct UpVal {
  HEAD;

  UpVal* next;
  bool   closed;

  union {
    Expr  val;
    Expr* loc;
  };
};

// standard representation for compile-time and run-time environments
struct Env {
  HEAD;

  Env*   parent;

  int    arity;
  int    ncap; // number of captured *local* upvalues

  EMap  vars; // personal namespace

  union {
    EMap  upvs;
    Exprs vals;
  };
};

// simple wrapper around a C file object
struct Port {
  HEAD;

  FILE* ios;
};

struct Fun {
  HEAD;

  Sym*   name;
  OpCode label;
  Chunk* chunk;
  Objs   upvs;
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
Expr  tag_obj(void* ptr);
void* mk_obj(ExpType type, flags_t flags);
void* clone_obj(void* ptr);
void  mark_obj(void* ptr);
void  unmark_obj(void* ptr);
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

// ref API
Ref* mk_ref(Sym* n, int o);

// upval APIs
UpVal* mk_upval(UpVal* next, Expr* loc);
Expr*  deref(UpVal* upv);

// environment API
Env* mk_env(Env* parent);
Ref* env_capture(Env* e, Ref* r);
Ref* env_resolve(Env* e, Sym* n, bool capture);
Ref* env_define(Env* e, Sym* n);
void toplevel_env_def(Env* e, Sym* n, Expr x);
void toplevel_env_set(Env* e, Sym* n, Expr x);
void toplevel_env_refset(Env* e, int n, Expr x);
Ref* toplevel_env_find(Env* e, Sym* n);
Expr toplevel_env_ref(Env* e, int n);
Expr toplevel_env_get(Env* e, Sym* n);

// port API -------------------------------------------------------------------
Port* mk_port(FILE* ios);
Port* open_port(char* fname, char* mode);
void  close_port(Port* port);
bool  peof(Port* p);
int   pseek(Port* p, long off, int orig);
Glyph pgetc(Port* p);
Glyph pungetc(Port* p, Glyph g);
Glyph ppeekc(Port* p);
int   pprintf(Port* p, char* fmt, ...);
int   pvprintf(Port* p, char* fmt, va_list va);

// function API ---------------------------------------------------------------
Fun* as_fun_s(char* f, Expr x);
Fun* mk_fun(Sym* name, OpCode op, Chunk* code);
Fun* mk_closure(Fun* proto);
Fun* mk_builtin_fun(Sym* name, OpCode op);
Fun* mk_user_fun(Chunk* code);
void def_builtin_fun(char* name, OpCode op);
void disassemble(Fun* fun);
Expr upval_ref(Fun* fun, int i);
void upval_set(Fun* fun, int i, Expr x);

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

// glyph API
Glyph as_glyph(Expr x);
Expr  tag_glyph(Glyph x);

// convenience macros
#define exp_tag(x)     ((x) & XTMSK)
#define exp_val(x)     ((x) & XVMSK)
#define head(x)        ((Obj*)as_obj(x))
#define as_fun(x)      ((Fun*)as_obj(x))
#define as_ref(x)      ((Ref*)as_obj(x))
#define as_env(x)      ((Env*)as_obj(x))
#define as_sym(x)      ((Sym*)as_obj(x))
#define as_str(x)      ((Str*)as_obj(x))
#define as_list(x)     ((List*)as_obj(x))

#define is_interned(s)    ((s)->flags == true)
#define is_keyword(s)     (*(s)->val->val == ':')
#define is_local_env(e)   ((e)->parent != NULL)
#define is_global_env(e)  ((e)->parent == NULL)
#define is_user_fn(f)     ((f)->label == OP_NOOP)
#define is_toplevel_fn(f) (!(f)->chunk->vars->local)
#define user_fn_argc(f)   ((f)->chunk->vars->arity)
#define user_fn_upvalc(f) ((f)->chunk->vars->upvs.count)
#define is_sym(x)         has_type(x, EXP_SYM)
#define is_fun(x)         has_type(x, EXP_FUN)
#define is_list(x)        has_type(x, EXP_LIST)

#endif
