#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "vm/vm.h"

// handling error state
ErrorState* error_state(RlState* rls);
void save_error_state(RlState* rls);
void restore_error_state(RlState* rls);
void discard_error_state(RlState* rls);

#define set_safe_point(rls) (setjmp((rls)->err_states[(rls)->ep-1].Cstate))
#define rl_longjmp(rls, s) longjmp(error_state(rls)->Cstate, s)

// error function
void rascal_error(RlState* rls, Status etype, char* fmt, ...);

// convenience macros
#define user_error(rls, args...) rascal_error(rls, USER_ERROR, args)
#define runtime_error(rls, args...) rascal_error(rls, RUNTIME_ERROR, args)
#define eval_error(rls, args...) rascal_error(rls, EVAL_ERROR, args)
#define system_error(rls, args...) rascal_error(rls, SYSTEM_ERROR, args)

#define require(rls, test, args...)             \
  do {                                          \
    if ( !(test) )                              \
      eval_error((rls), args);                  \
  } while ( false )

#define require_argco(rls, f, e, g)                                     \
  do {                                                                  \
    if ( (e) != (g) )                                                   \
      eval_error((rls), "%s wants %d inputs, got %d", (f), (e), (g));   \
  } while ( false )

#define require_argco2(rls, f, e1, e2, g)                               \
  do {                                                                  \
    if ( (e1) != (g) && (e2) != (g) )                                   \
      eval_error( (rls),                                                \
                  "%s wants %d or %d inputs, got %d",                   \
                  (f),                                                  \
                  (e1),                                                 \
                  (e2),                                                 \
                  (g));                                                 \
  } while ( false )

#define require_vargco(rls, f, e, g)                                    \
  do {                                                                  \
    if ( (g) < (e) )                                                    \
      eval_error( (rls), "%s wants at least %d inputs, got %d",         \
                  (f), (e), (g) );                                      \
  } while ( false )

#define require_argtype(rls, f, e, x)                                   \
  do {                                                                  \
    Type* _t = type_of(x);                                              \
    if ( (e)->tag != _t->tag )                                          \
      eval_error( (rls), "%s wanted a %s, got a %s",                    \
                  (f), type_name(e), type_name(_t));                    \
  } while ( false )

#define syntax_require(rls, sf, fn, t, fmt, ...)            \
  do {                                                      \
    if ( !(t) )                                             \
      eval_error( (rls),                                    \
                  "bad syntax for %s in %s: " fmt,          \
                  (sf), (fn) __VA_OPT__(,) __VA_ARGS__);    \
  } while ( false )

#define syntax_require_vargco(rls, sf, fn, e, f)                        \
  do {                                                                  \
    int __c = ((f) && (f)->tail) ? (int)((f)->tail->count) : -1;        \
    if ( __c < (e) )                                                    \
      eval_error( (rls),                                                \
                  "bad syntax for %s in %s: "                           \
                  "wanted at least %d expressions, got %d",             \
                  (sf), (fn), (e), __c );                               \
  } while ( false )

#define syntax_require_argco(rls, sf, fn, e, f)                         \
  do {                                                                  \
    int __c = ((f) && (f)->tail) ? (int)((f)->tail->count) : -1;        \
    if ( __c != (e) )                                                   \
      eval_error( (rls),                                                \
                  "bad syntax for %s in %s: "                           \
                  "wanted %d expressions, got %d",                      \
                  (sf), (fn), (e), __c );                               \
  } while ( false )

#define syntax_require_argco2(rls, sf, fn, e1, e2, f)                   \
  do {                                                                  \
    int __c = ((f) && (f)->tail) ? (int)((f)->tail->count) : -1;        \
    if ( __c != (e1) && __c != (e2) )                                   \
      eval_error( (rls),                                                \
                  "bad syntax for %s in %s: "                           \
                  "wanted %d or %d expressions, got %d",                \
                  (sf), (fn), (e1), (e2), __c );                        \
  } while ( false )

#endif
