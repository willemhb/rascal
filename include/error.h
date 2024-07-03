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
void  pr_prn_err(Proc* p);
Error pr_panic(Proc* p, Error e, Val b, const char* fmt, ...);
Error pr_recover(Proc* p, Sym** k, Str** m, Val* b);
Error pr_chk_type(Proc* p, Error e, Type* x, Val g, void* s);
Error pr_chk_argc(Proc* p, Error e, Func* fn, size_t g, bool v);
Error pr_chk_argt(Proc* p, Error e, Func* fn, size_t o, Val g);
Error pr_chk_bound(Proc* p, Error e, void* a, void* l, void* u);
Error pr_chk_lbound(Proc* p, Error e, void* a, void* l);
Error pr_chk_ubound(Proc* p, Error e, void* a, void* u);
Error pr_chk_push(Proc* p, Error e);
Error pr_chk_pushn(Proc* p, Error e, size_t n);
Error pr_chk_pop(Proc* p, Error e);
Error pr_chk_popn(Proc* p, Error e, size_t n);
Error pr_chk_stkn(Proc* p, Error e, size_t n);
Error pr_chk_sref(Proc* p, Error e, long i);
Error pr_chk_def(Proc* p, Error e, Ref* r);
Error pr_chk_refv(Proc* p, Error e, Ref* r);
Error pr_chk_reft(Proc* p, Error e, Ref* r, Val g);

/* Initialization */
void rl_error_init(void);

#endif
