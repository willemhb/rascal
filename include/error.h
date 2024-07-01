#ifndef rl_error_h
#define rl_error_h

#include "runtime.h"

/* Globals */
extern Sym* ErrorKwds[NUM_ERRORS];

/* Error APIs */
char* rl_err_name(Error e);
Sym*  rl_err_kwd(Error e);
Str*  rl_perror(Error e, const char* fn, const char* fmt, ...);
Str*  rl_vperror(Error e, const char* fn, const char* fmt, va_list va);
void  rl_fatal_err(Error e, const char* fn, const char* fmt, ...);

/* error checking helpers */
void   rlp_panic(RlProc* p, Error e, const char* fn, const char* fmt, ...);
void   rlp_argc(RlProc* p, Error e, const char* fn, size_t x, size_t g, bool v);
void   rlp_argt(RlProc* p, Error e, const char* fn, Type* x, Type* g);
void   rlp_bound(RlProc* p, Error e, const char* fn, void* a, void* l, void* u);
void   rlp_lbound(RlProc* p, Error e, const char* fn, void* a, void* l);
void   rlp_ubound(RlProc* p, Error e, const char* fn, void* a, void* u);

/* Initialization */
void rl_error_init(void);

#endif
