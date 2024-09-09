#ifndef rl_error_h
#define rl_error_h

#include "runtime.h"
#include "labels.h"

/* Globals */
extern Sym* ErrorKws[NUM_ERRS];

/* Error APIs */
char* rl_err_name(Error e);
Sym*  rl_err_kwd(Error e);
Str*  rl_err_str(Error e, const char* fn, const char* fmt, ...);
Str*  rl_verr_str(Error e, const char* fn, const char* fmt, va_list va);
void  rl_fatal_err(Error e, const char* fn, const char* fmt, ...);

/* error checking helpers */
void  prn_err(State* s);
Error panic(State* s, Error e, Val b, const char* fmt, ...);
Error recover(State* s, Sym** k, Str** m, Val* b);
Error chk_type(State* s, Error e, Type t, Val g);
Error chk_argc(State* s, Error e, Func* fn, size_t g, bool v);
Error chk_argt(State* s, Error e, Func* fn, size_t o, Val g);
Error chk_bound(State* s, Error e, void* a, void* l, void* u);
Error chk_lbound(State* s, Error e, void* a, void* l);
Error chk_ubound(State* s, Error e, void* a, void* u);
Error chk_push(State* s, Error e);
Error chk_pushn(State* s, Error e, size_t n);
Error chk_pop(State* s, Error e);
Error chk_popn(State* s, Error e, size_t n);
Error chk_stkn(State* s, Error e, size_t n);
Error chk_sref(State* s, Error e, long i);

/* Initialization */
void rl_error_init(void);

#endif
