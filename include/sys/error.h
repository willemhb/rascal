#ifndef rl_sys_error_h
#define rl_sys_error_h

/**
 *
 * Rascal builtin error handling support.
 * 
 **/

// headers --------------------------------------------------------------------
#include "sys/base.h"

// macros ---------------------------------------------------------------------
#define safepoint() setjmp(SaveState)

#define user_error(args...)    rascal_error(USER_ERROR, args)
#define runtime_error(args...) rascal_error(RUNTIME_ERROR, args)
#define eval_error(args...)    rascal_error(EVAL_ERROR, args)
#define system_error(args...)  rascal_error(SYSTEM_ERROR, args)

#define require(test, args...)                  \
  do {                                          \
    if ( !(test) )                              \
      eval_error(args);                         \
  } while ( false )

#define require_argco(f, e, g)                                          \
  do {                                                                  \
    if ( (e) != (g) )                                                   \
      eval_error( "%s wants %d inputs, got %d", (f), (e), (g) );        \
  } while ( false )

#define require_argco2(f, e1, e2, g)                                    \
  do {                                                                  \
    if ( (e1) != (g) && (e2) != (g) )                                   \
      eval_error( "%s wants %d or %d inputs, got %d",                   \
                  (f),                                                  \
                  (e1),                                                 \
                  (e2),                                                 \
                  (g));                                                 \
  } while ( false )

#define require_vargco(f, e, g)                                         \
  do {                                                                  \
    if ( (g) < (e) )                                                    \
      eval_error( "%s wants at least %d inputs, got %d",                \
                  (f), (e), (g) );                                      \
  } while ( false )

#define require_argtype(f, e, x)                                        \
  do {                                                                  \
    ExpType _t = exp_type(x);                                           \
    if ( (e) != _t )                                                    \
      eval_error("%s wanted a %s, got a %s",                            \
                 (f), Types[e].name, Types[_t].name);                   \
  } while ( false )

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
// high level error helpers ---------------------------------------------------
void panic(Status etype);
void recover(funcptr_t cleanup);
void rascal_error(Status etype, char* fmt, ...);

// initialization -------------------------------------------------------------
void toplevel_init_sys_error(void);

#endif
