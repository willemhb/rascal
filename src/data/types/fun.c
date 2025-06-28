/* Common function representation (includes builtin and user functions).

   There is no separate closure type for the time being. */
// headers --------------------------------------------------------------------
#include "data/types/fun.h"
#include "data/types/sym.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_fun(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void trace_fun(void* ptr) {
  Fun* fun = ptr;

  mark_obj(fun->name);
  mark_obj(fun->chunk);
}

// external -------------------------------------------------------------------
Fun* as_fun_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_FUN, "%s wanted a fun, got %s", f, Types[t].name);

  return as_fun(x);
}

Fun* mk_fun(Sym* name, OpCode op, Chunk* code) {
  Fun* f   = mk_obj(EXP_FUN, 0);
  f->name  = name;
  f->label = op;
  f->chunk = code;

  init_objs(&f->upvs);

  return f;
}

Fun* mk_closure(Fun* proto) {
  Fun* cls; int count = user_fn_upvalc(proto);

  if ( count == 0 )
    cls = proto;

  else {
    cls = clone_obj(proto);
    objs_write(&cls->upvs, NULL, count);
  }

  return cls;
}

Fun* mk_builtin_fun(Sym* name, OpCode op) {
  return mk_fun(name, op, NULL);
}

Fun* mk_user_fun(Chunk* code) {
  Sym* n = mk_sym("fn"); preserve(1, tag_obj(n));
  Fun* f = mk_fun(n, OP_NOOP, code);

  return f;
}

void disassemble(Fun* fun) {
  printf("\n\n==== %s ====\n\n", fun->name->val->val);
  dis_chunk(fun->chunk);
  printf("\n\n");
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_fun(void) {
  Types[EXP_FUN] = (ExpTypeInfo) {
    .type     = EXP_FUN,
    .name     = "fun",
    .obsize   = sizeof(Fun),
    .trace_fn = trace_fun
  };
}
