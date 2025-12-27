#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "vm/vm.h"
#include "val/ctl.h"

// handling error state
Ctl* save_error_state(RlState* rls, int o);
Ctl* get_error_state(RlState* rls);
Ctl* restore_error_state(RlState* rls);
Ctl* discard_error_state(RlState* rls);

#define rl_setjmp(rls)     setjmp(get_error_state(rls)->Cstate)
#define rl_longjmp(rls, s) longjmp(get_error_state(rls)->Cstate, s)

// error functions
void rascal_error(RlState* rls, Status etype, bool fatal, Str* message);

void runtime_error(RlState* rls, char* fmt, ...);
void eval_error(RlState* rls, char* fmt, ...);
void type_error(RlState* rls, Type* e, Expr g);
void arity_error(RlState* rls, bool va, int e, int g);
void syntax_error(RlState* rls, List* form, char* fmt, ...);
void syntax_type_error(RlState* rls, List* form, Type* e, Expr g);
void syntax_arity_error(RlState* rls, List* form, bool va, int e);
void system_error(RlState* rls, char* fmt, ...);
void fatal_error(RlState* rls, char* fmt, ...);

// require helpers
#define system_require(rls, t, args...)                         \
  do {                                                          \
    if ( !(t) )                                                 \
      system_error((rls), args);                                \
  } while ( false )

#define fatal_require(rls, t, args...)                          \
  do {                                                          \
    if ( !(t) )                                                 \
      fatal_error((rls), args);                                 \
  } while ( false )

#define runtime_require(rls, t, args...)        \
  do {                                          \
    if ( !(t) )                                 \
      runtime_error((rls), args);               \
  } while ( false )

#define require(rls, t, args...)                \
  do {                                          \
    if ( !(t) )                                 \
      eval_error((rls), args);                  \
  } while ( false )

#define require_argco(rls, e, g)                \
  do {                                          \
    if ( (e) != (g) )                           \
      arity_error((rls), false, (e), (g));      \
  } while ( false )

#define require_vargco(rls, e, g)               \
  do {                                          \
    if ( (g) < (e) )                            \
      arity_error((rls), true, (e), (g));       \
  } while ( false )

#define require_argtype(rls, e, g)              \
  do {                                          \
    if ( !has_type(g, e) )                      \
      type_error(rls, e, g);                    \
  } while ( false )

#define syntax_require(rls, t, sf, args...)     \
  do {                                          \
    if ( !(t) )                                 \
      syntax_error(rls, sf, args);              \
  } while ( false )

#define syntax_require_argco(rls, sf, e)        \
  do {                                          \
    if ( (sf)->count - 1 != (e) )               \
      syntax_arity_error(rls, sf, false, e);    \
  } while ( false )

#define syntax_require_vargco(rls, sf, e)       \
  do {                                          \
    if ( (sf)->count - 1 < (e) )                \
      syntax_arity_error(rls, sf, true, e);     \
  } while ( false )

#define syntax_require_argtype(rls, sf, e, g)   \
  do {                                          \
    if ( !has_type(g, e) )                      \
      syntax_type_error(rls, sf, e, g);         \
  } while ( false )

// miscellaneous helpers
void print_stack_trace(RlState* rls);
char* error_name(Status etype);
Sym* error_sym(Status etype);
void init_vm_error(void);

#endif
