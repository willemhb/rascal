#ifndef rl_fun_h
#define rl_fun_h

#include "val/val.h"
#include "val/env.h"
#include "val/sym.h"
#include "util/collection.h"

struct Chunk {
  HEAD;
  Exprs vals;
  CodeBuf code;
  LineInfo lines;
  Env* vars;
  Sym* name;
  Str* file;
};

// represents a callable function (with possible overloads)
struct Fun {
  HEAD;
  Sym* name;
  bool macro;
  bool generic;
  int mcount;

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
  UpVal** upvs;
};

struct MethodTable {
  HEAD;
  Fun* fun; // the function object this is a method table for
  Method* variadic; // only one variadic method is currently supported
  BitVec methods; // bit vector of fixed arity methods
};

// inline helpers
static inline bool argc_match(Method* m, int argc) {
  if ( m->va )
    return argc >= m->arity;

  return argc == m->arity;
}

static inline int mtable_count(MethodTable* m) {
  return m->methods.count + (m->variadic != NULL);    
}

// chunk API
Chunk* mk_chunk(RlState* rls, Env* penv, Sym* name, Str* file);
Chunk* mk_chunk_s(RlState* rls, Env* penv, Sym* name, Str* file);
int get_line_number(Chunk* c, instr_t* off);
void add_line_number(RlState* rls, Chunk* c, int line);
void finalize_chunk(RlState* rls, Chunk* c, int line);
void disassemble_chunk(Chunk* chunk);

// function API
Fun* mk_fun(RlState* rls, Sym* name, bool macro, bool generic);
Fun* mk_fun_s(RlState* rls, Sym* name, bool macro, bool generic);
void fun_add_method(RlState* rls, Fun* fun, Method* m);
void fun_add_method_s(RlState* rls, Fun* fun, Method* m);
Method* fun_get_method(Fun* fun, int argc);
Fun* def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op);
void add_builtin_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op);
char* get_fn_name(void* ob, char* fallback);

// method API
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

// convenience macros and accessors
#define as_method(x)     ((Method*)as_obj(x))
#define as_fun(x)        ((Fun*)as_obj(x))
#define as_mtable(x)     ((MethodTable*)as_obj(x))
#define as_fun_s(rls, x) ((Fun*)as_obj_s(rls, &FunType, x))

#define chunk_code(c)     ((c)->code.data)
#define chunk_codec(c)    ((c)->code.count)
#define chunk_vals(c)     ((c)->vals.data)
#define chunk_valsc(c)    ((c)->vals.count)
#define chunk_lines(c)    ((c)->lines.data)
#define chunk_linesc(c)   ((c)->lines.count)
#define chunk_name(c)     ((c)->name->val->val)
#define chunk_file(c)     ((c)->file->val)
#define chunk_upvc(c)     (env_upvc((c)->vars))

#define is_user_method(m)    ((m)->label == OP_NOOP)
#define is_builtin_method(m) ((m)->label != OP_NOOP)
#define is_singleton_fn(f)   ((f)->mcount == 1)
#define fn_name(f)           ((f)->name->val->val)
#define method_argc(m)       ((m)->arity)
#define method_va(m)         ((m)->va)
#define method_name(m)       ((m)->fun->name->val->val)
#define mtable_name(m)       ((m)->fun->name->val->val)
#define method_upvs(m)       ((m)->upvs)

static inline int method_formalc(Method* m) {
  return m->arity + m->va;
}

static inline int method_upvc(Method* m) {
  return is_builtin_method(m) ? -1 : env_upvc(m->chunk->vars);
}

static inline char* method_file(Method* m) {
  return is_builtin_method(m) ? "<builtin>" : m->chunk->file->val;
}

static inline Expr* method_vals(Method* m) {
  return is_builtin_method(m) ? NULL : m->chunk->vals.data;
}

static inline instr_t* method_code(Method* m) {
  return is_builtin_method(m) ? NULL : m->chunk->code.data;
}

#endif

