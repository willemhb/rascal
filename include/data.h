#ifndef rl_data_h
#define rl_data_h

#include <stdio.h>

#include "common.h"
#include "collection.h"
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

typedef enum {
  // memory flags
  FL_BLACK    = 0x8000,
  FL_GRAY     = 0x4000,
  FL_NOSWEEP  = 0x2000,
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
  bool macro; // true if this ref points to a macro
  bool final; // true if this ref can't be reassigned (initial value is always NONE)
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
  int ncap; // number of captured *local* upvalues

  EMap vars; // personal namespace

  union {
    EMap upvs;
    Objs vals;
  };
};

// simple wrapper around a C file object
typedef enum {
  INPUT_PORT=0x01,
  OUTPUT_PORT=0x02,
  TEXT_PORT=0x04,
  BINARY_PORT=0x08,
  LISP_PORT=0x10,
} IOMode;

#define IO_INPUT_MASK 0x03
#define IO_PTYPE_MASK 0x0c
#define IO_LISP_MASK  0x10

struct Port {
  HEAD;

  IOMode mode;

  FILE* ios;
};

// represents a callable function (with possible overloads)
struct Fun {
  HEAD;
  Sym* name;
  bool macro;
  bool generic;

  union {
    MethodTable* methods;
    Method* method; // for singleton
  };
};

// represents a single callable function
struct Method {
  HEAD;
  Fun* fun; // the function this method belongs to
  bool va; // whether extra arguments are accepted
  int arity; // number of arguments accepted (if va is true, this is the minimum number of arguments)
  OpCode label; // for builtins, set to OP_NOOP for user methods
  Chunk* chunk; // bytecode for closures
  Objs upvs; // captured upvalues for closures
};

static inline bool argc_match(Method* m, int argc) {
  if ( m->va )
    return argc >= m->arity;

  return argc == m->arity;
}

struct MethodTable {
  HEAD;
  Fun* fun; // the function object this is a method table for
  uintptr_t bitmap; // bitmap of used arities (maps arity to actual index in methods)
  Method* variadic; // only one variadic method is currently supported
  Objs methods; // the stored methods
};

static inline int fn_method_count(Fun* fn) {
  if ( fn->method == NULL )
    return 0;

  else if ( fn->method->type->tag == EXP_METHOD )
    return 1;

  else
    return fn->methods->methods.count;
}

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

// chunk API
Chunk* mk_chunk(RlState* rls, Env* vars, Alist* vals, Buf16* code);
Chunk* mk_chunk_s(RlState* rls, Env* vars, Alist* vals, Buf16* code);
void do_disassemble(Env* vars, Alist* vals, Buf16* code);
void disassemble_chunk(Chunk* chunk);

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
void free_buf16(RlState* rls, void* ptr);
int buf16_write(RlState* rls, Buf16* b, ushort_t *xs, int n);

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
Ref*   env_define(RlState* rls, Env* e, Sym* n, bool m, bool f, bool *a);
void   toplevel_env_def(RlState* rls, Env* e, Sym* n, Expr x, bool m, bool f);
void   toplevel_env_set(RlState* rls, Env* e, Sym* n, Expr x);
void   toplevel_env_refset(RlState* rls, Env* e, int n, Expr x);
Ref*   toplevel_env_find(RlState* rls, Env* e, Sym* n);
Expr   toplevel_env_ref(RlState* rls, Env* e, int n);
Expr   toplevel_env_get(RlState* rls, Env* e, Sym* n);

// port API -------------------------------------------------------------------
Port* mk_port(RlState* rls, FILE* ios, IOMode io_mode);
Port* mk_port_s(RlState* rls, FILE* ios, IOMode io_mode);
Port* open_port(RlState* rls, char* fname, char* mode);
Port* open_port_s(RlState* rls, char* fname, char* mode);
void close_port(Port* port);
bool peof(Port* p);
int pseek(Port* p, long off, int orig);
Glyph pgetc(Port* p);
Glyph pungetc(Port* p, Glyph g);
Glyph ppeekc(Port* p);
int pprintf(Port* p, char* fmt, ...);
int pvprintf(Port* p, char* fmt, va_list va);

// function API ---------------------------------------------------------------
Fun* mk_fun(RlState* rls, Sym* name, bool macro, bool generic);
Fun* mk_fun_s(RlState* rls, Sym* name, bool macro, bool generic);
void fun_add_method(RlState* rls, Fun* fun, Method* m);
void fun_add_method_s(RlState* rls, Fun* fun, Method* m);
Method* fun_get_method(Fun* fun, int argc);
Fun* def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op);
char* get_fn_name(void* ob, char* fallback);

// method API -----------------------------------------------------------------
Method* mk_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op, Chunk* code);
Method* mk_method_s(RlState* rls, Fun* fun, int arity, bool va, OpCode op, Chunk* code);
Method* mk_closure(RlState* rls, Method* proto);
Method* mk_builtin_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op);
Method* mk_builtin_method_s(RlState* rls, Fun* fun, int arity, bool va, OpCode op);
Method* mk_user_method(RlState* rls, Fun* fun, int arity, bool va, Chunk* code);
Method* mk_user_method_s(RlState* rls, Fun* fun, int arity, bool va, Chunk* code);
void    disassemble_method(Method* m);
Expr    upval_ref(Method* m, int i);
void    upval_set(Method* m, int i, Expr x);

// method table API
MethodTable* mk_mtable(RlState* rls, Fun* fun);
MethodTable* mk_mtable_s(RlState* rls, Fun* fun);
void mtable_add(RlState* rls, MethodTable* mt, Method* m);
Method* mtable_lookup(MethodTable* mt, int argc);

// symbol API
Sym* mk_sym(RlState* rls, char* cs);
Sym* mk_sym_s(RlState* rls, char* cs);
Sym* mk_module_name_s(RlState* rls, char* fname);
bool sym_val_eql(Sym* s, char* v);

// string API
Str* mk_str(RlState* rls, char* cs);
Str* mk_str_s(RlState* rls, char* cs);

// list API
List*  empty_list(RlState* rls);
List*  empty_list_s(RlState* rls);
List*  mk_list(RlState* rls, size_t n, Expr* xs);
List*  mk_list_s(RlState* rls, size_t n, Expr* xs);
List*  cons(RlState* rls, Expr hd, List* tl);
List*  cons_s(RlState* rls, Expr hd, List* tl);
List*  cons_n(RlState* rls, int n);
List*  cons_n_s(RlState* rls, int n);
Expr   list_ref(List* xs, int n);
int    push_list(RlState* rls, List* xs);

// number API
Num as_num_s(RlState* rls, char* f, Expr x);
Num as_num(Expr x);
Expr tag_num(Num n);
uintptr_t as_fix(Expr x);
Expr tag_fix(uintptr_t i);
void* as_ptr(Expr x);
Expr tag_ptr(void* ptr);

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
#define as_type(x)     ((Type*)as_obj(x))
#define as_port(x)     ((Port*)as_obj(x))
#define as_method(x)   ((Method*)as_obj(x))
#define as_fun(x)      ((Fun*)as_obj(x))
#define as_mtable(x)   ((MethodTable*)as_obj(x))
#define as_ref(x)      ((Ref*)as_obj(x))
#define as_env(x)      ((Env*)as_obj(x))
#define as_sym(x)      ((Sym*)as_obj(x))
#define as_str(x)      ((Str*)as_obj(x))
#define as_list(x)     ((List*)as_obj(x))

#define as_type_s(rls, f, x) ((Type*)as_obj_s(rls, f, &TypeType, x))
#define as_sym_s(rls, f, x)  ((Sym*)as_obj_s(rls, f, &SymType, x))
#define as_list_s(rls, f, x) ((List*)as_obj_s(rls, f, &ListType, x))
#define as_str_s(rls, f, x)  ((Str*)as_obj_s(rls, f, &StrType, x))
#define as_env_s(rls, f, x)  ((Env*)as_obj_s(rls, f, &EnvType, x))
#define as_port_s(rls, f, x) ((Port*)as_obj_s(rls, f, &PortType, x))
#define as_fun_s(rls, f, x)  ((Fun*)as_obj_s(rls, f, &FunType, x))

#define is_interned(s)     ((s)->flags == true)
#define is_keyword(s)      (*(s)->val->val == ':')
#define is_local_env(e)    ((e)->parent != NULL)
#define is_global_env(e)   ((e)->parent == NULL)
#define is_user_method(m)  ((m)->label == OP_NOOP)
#define is_singleton_fn(f) ((f)->method->type->tag == EXP_METHOD)
#define fn_name(f)         ((f)->name->val->val)
#define method_argc(m)     ((m)->arity)
#define method_va(m)       ((m)->va)
#define method_upvalc(m)   ((m)->chunk->vars->upvs.count)
#define method_name(m)     ((m)->fun->name->val->val)
#define mtable_name(m)     ((m)->fun->name->val->val)
#define mtable_count(m)    ((m)->methods.count)
#define sym_val(s)         ((s)->val->val)
#define env_type(e)        ((e)->etype)
#define env_size(e)        ((e)->vars.count)

extern Type GlyphType, NumType, SymType, MethodType, FunType,
  MethodTableType, ListType, EnvType, StrType, TypeType;

#define is_glyph(x)       has_type(x, &GlyphType)
#define is_num(x)         has_type(x, &NumType)
#define is_sym(x)         has_type(x, &SymType)
#define is_method(x)      has_type(x, &MethodType)
#define is_fun(x)         has_type(x, &FunType)
#define is_mtable(x)      has_type(x, &MethodTableType)
#define is_list(x)        has_type(x, &ListType)

#define list_fst(l)       ((l)->head)
#define list_snd(l)       ((l)->tail->head)
#define list_thd(l)       ((l)->tail->tail->head)
#define list_fth(l)       ((l)->tail->tail->tail->head)

#endif
