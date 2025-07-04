#ifndef rl_data_fun_h
#define rl_data_fun_h

/* Common function representation (includes builtin and user functions).

   There is no separate closure type for the time being. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

#include "sys/opcode.h"

// macros ---------------------------------------------------------------------
#define as_fun(x)         ((Fun*)as_obj(x))
#define is_fun(x)         has_type(x, EXP_FUN)
#define is_user_fn(f)     ((f)->label == OP_NOOP)
#define is_toplevel_fn(f) (!(f)->chunk->vars->local)
#define user_fn_argc(f)   ((f)->chunk->vars->arity)
#define user_fn_upvalc(f) ((f)->chunk->vars->upvs.count)
#define user_fn_instr(f)  ((f)->chunk->code->binary.vals)

// C types --------------------------------------------------------------------
struct Fun {
  HEAD;

  Sym*   name;
  OpCode label;
  Chunk* chunk;
  Objs   upvs;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Fun* as_fun_s(char* f, Expr x);
Fun* mk_fun(Sym* name, OpCode op, Chunk* code);
Fun* mk_closure(Fun* proto);
Fun* mk_builtin_fun(Sym* name, OpCode op);
Fun* mk_user_fun(Chunk* code);
void disassemble(Fun* fun);

// initialization -------------------------------------------------------------
void toplevel_init_data_fun(void);

#endif
