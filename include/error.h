#ifndef rl_error_h
#define rl_error_h

#include "runtime.h"

/* Globals */
extern Sym* ErrorKwds[NUM_ERRORS];

/* Error APIs */
char* rl_err_name(Error e);
Sym*  rl_err_kwd(Error e);
Str*  rl_err_str(Error e, const char* fn, const char* fmt, ...);
Str*  rl_verr_str(Error e, const char* fn, const char* fmt, va_list va);
void  rl_fatal_err(Error e, const char* fn, const char* fmt, ...);

/* error checking helpers */
void  rlp_prn_err(RlProc* p);
Error rlp_panic(RlProc* p, Error e, Val b, const char* fmt, ...);
Error rlp_recover(RlProc* p, Sym** k, Str** m, Val* b);
Error rlp_chk_type(RlProc* p, Error e, Type* x, Val g, void* s);
Error rlp_chk_argc(RlProc* p, Error e, Func* fn, size_t g, bool v);
Error rlp_chk_argt(RlProc* p, Error e, Func* fn, size_t o, Val g);
Error rlp_chk_bound(RlProc* p, Error e, void* a, void* l, void* u);
Error rlp_chk_lbound(RlProc* p, Error e, void* a, void* l);
Error rlp_chk_ubound(RlProc* p, Error e, void* a, void* u);
Error rlp_chk_grows(RlProc* p, Error e, size_t n);
Error rlp_chk_push(RlProc* p, Error e);
Error rlp_chk_shrinks(RlProc* p, Error e, size_t n);
Error rlp_chk_pop(RlProc* p, Error e);
Error rlp_chk_sref(RlProc* p, Error e, long i);
Error rlp_chk_def(RlProc* p, Error e, Ref* r);
Error rlp_chk_refv(RlProc* p, Error e, Ref* r);
Error rlp_chk_reft(RlProc* p, Error e, Ref* r, Val g);

/* Initialization */
void rl_error_init(void);

#endif
