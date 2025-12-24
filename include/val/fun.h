#ifndef rl_fun_h
#define rl_fun_h

#include "val/val.h"
#include "val/env.h"
#include "val/alist.h"
#include "val/buf.h"
#include "val/sym.h"

struct Chunk {
  HEAD;
  Env*    vars;
  Alist*  vals;
  Buf16*  code;
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

struct MethodTable {
  HEAD;
  Fun* fun; // the function object this is a method table for
  uintptr_t bitmap; // bitmap of used arities (maps arity to actual index in methods)
  Method* variadic; // only one variadic method is currently supported
  Objs methods; // the stored methods
};

// inline helpers
static inline bool argc_match(Method* m, int argc) {
  if ( m->va )
    return argc >= m->arity;

  return argc == m->arity;
}

static inline int fn_method_count(Fun* fn) {
  if ( fn->method == NULL )
    return 0;

  else if ( fn->method->type->tag == EXP_METHOD )
    return 1;

  else
    return fn->methods->methods.count;
}

// chunk API
Chunk* mk_chunk(RlState* rls, Env* vars, Alist* vals, Buf16* code);
Chunk* mk_chunk_s(RlState* rls, Env* vars, Alist* vals, Buf16* code);
void do_disassemble(Env* vars, Alist* vals, Buf16* code);
void disassemble_chunk(Chunk* chunk);

// function API
Fun* mk_fun(RlState* rls, Sym* name, bool macro, bool generic);
Fun* mk_fun_s(RlState* rls, Sym* name, bool macro, bool generic);
void fun_add_method(RlState* rls, Fun* fun, Method* m);
void fun_add_method_s(RlState* rls, Fun* fun, Method* m);
Method* fun_get_method(Fun* fun, int argc);
Fun* def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op);
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

// convenience macros
#define as_method(x)   ((Method*)as_obj(x))
#define as_fun(x)      ((Fun*)as_obj(x))
#define as_mtable(x)   ((MethodTable*)as_obj(x))
#define as_fun_s(rls, f, x)  ((Fun*)as_obj_s(rls, f, &FunType, x))

#define is_user_method(m)  ((m)->label == OP_NOOP)
#define is_singleton_fn(f) ((f)->method->type->tag == EXP_METHOD)
#define fn_name(f)         ((f)->name->val->val)
#define method_argc(m)     ((m)->arity)
#define method_va(m)       ((m)->va)
#define method_upvalc(m)   ((m)->chunk->vars->upvs.count)
#define method_name(m)     ((m)->fun->name->val->val)
#define mtable_name(m)     ((m)->fun->name->val->val)
#define mtable_count(m)    ((m)->methods.count)

#endif
