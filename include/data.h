#ifndef rl_data_h
#define rl_data_h

#include <stdio.h>

#include "common.h"
#include "collection.h"
#include "opcode.h"

// Types
// Expression type codes
typedef enum {
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
  EXP_METHOD,
  EXP_FUN,
  EXP_MTABLE,
  EXP_SYM,
  EXP_STR,
  EXP_LIST,
  EXP_NUM
} ExpType;

#define HEAD                                     \
  Obj* heap;                                     \
  Type* type;                                    \
  union {                                        \
    flags_t bfields;                             \
    struct {                                     \
      flags_t black   :   1;                     \
      flags_t gray    :   1;                     \
      flags_t nosweep :   1;                     \
      flags_t flags    : 29;                     \
    };                                           \
  }

// Type object - first class type representation
struct Type {
  HEAD;

  ExpType tag; // for easy type comparison and hashing
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

typedef enum {
  FL_BLACK   = 0x80000000,
  FL_GRAY    = 0x40000000,
  FL_NOSWEEP = 0x20000000
} ExpFlags;

struct Obj {
  HEAD;
};

// Array and table types are now declared in collection.h

void trace_exprs(RlState* rls, Exprs* xs);
void trace_objs(RlState* rls, Objs* os);

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
} RefCode;

struct Ref {
  HEAD;

  Ref* captures;
  Sym* name;
  RefCode ref_type;
  int offset;
  bool is_macro; // true if this ref points to a macro
  Expr val; // only used at Global scope
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

  Env* parent;
  RefCode etype;
  int arity;
  bool va;
  int ncap; // number of captured *local* upvalues

  EMap vars; // personal namespace

  union {
    EMap upvs;
    Objs vals;
  };
};

// simple wrapper around a C file object
struct Port {
  HEAD;

  FILE* ios;
};

// represents a single callable function
struct Method {
  HEAD;
  int arity;
  bool va;
  Sym* name;
  OpCode label;
  Chunk* chunk;
  Objs upvs;
};

struct MethodTable {
  HEAD;
  Method* variadic;
  Objs methods;
};

struct Fun {
  HEAD;
  Sym* name;
  int num_methods;
  Method* singleton;
  MethodTable* methods;
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

// forward declarations
// type API
void init_builtin_type(RlState* rls, Type* type, char* name);
void register_builtin_types(RlState* rls);
Type* type_of(Expr x);
bool has_type(Expr x, Type* t);
char* type_name(Type* t);
ExpType exp_type(Expr x);

// expression APIs
hash_t hash_exp(Expr x);
bool egal_exps(Expr x, Expr y);
void mark_exp(RlState* rls, Expr x);

// object API
void* as_obj(Expr x);
Expr  tag_obj(void* ptr);
void* mk_obj(RlState* rls, Type* type, flags_t flags);
void* mk_obj_s(RlState* rls, Type* type, flags_t flags);
void* clone_obj(RlState* rls, void* ptr);
void* clone_obj_s(RlState* rls, void* ptr);
void  mark_obj(RlState* rls, void* ptr);
void  unmark_obj(void* ptr);
void  free_obj(RlState* rls, void *ptr);

// chunk API
Chunk* mk_chunk(RlState* rls, Env* vars, Alist* vals, Buf16* code);
Chunk* mk_chunk_s(RlState* rls, Env* vars, Alist* vals, Buf16* code);
void   dis_chunk(Chunk* chunk);

// alist API
Alist* mk_alist(RlState* rls);
Alist* mk_alist_s(RlState* rls);
void   free_alist(RlState* rls, void* ptr);
int    alist_push(RlState* rls, Alist* a, Expr x);
Expr   alist_pop(RlState* rls, Alist* a);
Expr   alist_get(Alist* a, int n);

// buf16 API
Buf16* mk_buf16(RlState* rls);
Buf16* mk_buf16_s(RlState* rls);
void   free_buf16(RlState* rls, void* ptr);
int    buf16_write(RlState* rls, Buf16* b, ushort_t *xs, int n);

// ref API
Ref* mk_ref(RlState* rls, Sym* n, int o);

// upval APIs
UpVal* mk_upval(RlState* rls, UpVal* next, Expr* loc);
Expr*  deref(UpVal* upv);

// environment API
Env*   mk_env(RlState* rls, Env* parent);
Env*   mk_env_s(RlState* rls, Env* parent);
Ref*   env_capture(RlState* rls, Env* e, Ref* r);
Ref*   env_resolve(RlState* rls, Env* e, Sym* n, bool capture);
Ref*   env_define(RlState* rls, Env* e, Sym* n);
void   toplevel_env_def(RlState* rls, Env* e, Sym* n, Expr x);
void   toplevel_env_set(RlState* rls, Env* e, Sym* n, Expr x);
void   toplevel_env_refset(RlState* rls, Env* e, int n, Expr x);
Ref*   toplevel_env_find(RlState* rls, Env* e, Sym* n);
Expr   toplevel_env_ref(RlState* rls, Env* e, int n);
Expr   toplevel_env_get(RlState* rls, Env* e, Sym* n);

// port API -------------------------------------------------------------------
Port*  mk_port(RlState* rls, FILE* ios);
Port*  mk_port_s(RlState* rls, FILE* ios);
Port*  open_port(RlState* rls, char* fname, char* mode);
Port*  open_port_s(RlState* rls, char* fname, char* mode);
void   close_port(Port* port);
bool  peof(Port* p);
int   pseek(Port* p, long off, int orig);
Glyph pgetc(Port* p);
Glyph pungetc(Port* p, Glyph g);
Glyph ppeekc(Port* p);
int   pprintf(Port* p, char* fmt, ...);
int   pvprintf(Port* p, char* fmt, va_list va);

// method API -----------------------------------------------------------------
Method* mk_method(RlState* rls, Sym* name, int arity, bool va, OpCode op, Chunk* code);
Method* mk_method_s(RlState* rls, Sym* name, int arity, bool va, OpCode op, Chunk* code);
Method* mk_closure(RlState* rls, Method* proto);
Method* mk_builtin_method(RlState* rls, Sym* name, int arity, bool va, OpCode op);
Method* mk_user_method(RlState* rls, Chunk* code);
Method* mk_user_method_s(RlState* rls, Chunk* code);
void    disassemble(Method* m);
Expr    upval_ref(Method* m, int i);
void    upval_set(Method* m, int i, Expr x);

// method table API
MethodTable* mk_mtable(RlState* rls);
MethodTable* mk_mtable_s(RlState* rls);
void         mtable_add(RlState* rls, MethodTable* mt, Method* m);
Method*      mtable_lookup(MethodTable* mt, int argc);

// function API ---------------------------------------------------------------
Fun*    as_fun_s(RlState* rls, char* f, Expr x);
Fun*    mk_fun(RlState* rls, Sym* name, Method* m);
Fun*    mk_fun_s(RlState* rls, Sym* name, Method* m);
Fun*    mk_user_fun(RlState* rls, Chunk* code);
Fun*    mk_user_fun_s(RlState* rls, Chunk* code);
void    fun_add_method(RlState* rls, Fun* fun, Method* m);
Method* fun_lookup(Fun* fun, int argc);
void    def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op);

// symbol API
Sym*   as_sym_s(RlState* rls, char* f, Expr x);
Sym*   mk_sym(RlState* rls, char* cs);
Sym*   mk_sym_s(RlState* rls, char* cs);
bool   sym_val_eql(Sym* s, char* v);

// string API
Str*   as_str_s(RlState* rls, char* f, Expr x);
Str*   mk_str(RlState* rls, char* cs);
Str*   mk_str_s(RlState* rls, char* cs);

// list API
List*  as_list_s(RlState* rls, char* f, Expr x);
List*  empty_list(RlState* rls);
List*  empty_list_s(RlState* rls);
List*  mk_list(RlState* rls, size_t n, Expr* xs);
List*  mk_list_s(RlState* rls, size_t n, Expr* xs);
List*  cons(RlState* rls, Expr hd, List* tl);
List*  cons_s(RlState* rls, Expr hd, List* tl);
List*  cons_n(RlState* rls, size_t n);
List*  cons_n_s(RlState* rls, size_t n);
Expr   list_ref(List* xs, int n);

// number API
Num       as_num_s(RlState* rls, char* f, Expr x);
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
Glyph as_glyph_s(RlState* rls, char* f, Expr x);
Expr  tag_glyph(Glyph x);

// convenience macros
#define exp_tag(x)     ((x) & XTMSK)
#define exp_val(x)     ((x) & XVMSK)
#define head(x)        ((Obj*)as_obj(x))
#define as_method(x)   ((Method*)as_obj(x))
#define as_fun(x)      ((Fun*)as_obj(x))
#define as_mtable(x)   ((MethodTable*)as_obj(x))
#define as_ref(x)      ((Ref*)as_obj(x))
#define as_env(x)      ((Env*)as_obj(x))
#define as_sym(x)      ((Sym*)as_obj(x))
#define as_str(x)      ((Str*)as_obj(x))
#define as_list(x)     ((List*)as_obj(x))

#define is_interned(s)    ((s)->flags == true)
#define is_keyword(s)     (*(s)->val->val == ':')
#define is_local_env(e)   ((e)->parent != NULL)
#define is_global_env(e)  ((e)->parent == NULL)
#define is_user_method(m) ((m)->label == OP_NOOP)
#define method_argc(m)    ((m)->arity)
#define method_va(m)      ((m)->va)
#define method_upvalc(m)  ((m)->chunk->vars->upvs.count)
#define env_type(e)       ((e)->etype)
#define env_size(e)       ((e)->vars.count)

extern Type GlyphType, NumType, SymType, MethodType, FunType, MethodTableType, ListType;

#define is_glyph(x)       has_type(x, &GlyphType)
#define is_num(x)         has_type(x, &NumType)
#define is_sym(x)         has_type(x, &SymType)
#define is_method(x)      has_type(x, &MethodType)
#define is_fun(x)         has_type(x, &FunType)
#define is_mtable(x)      has_type(x, &MethodTableType)
#define is_list(x)        has_type(x, &ListType)

#endif
