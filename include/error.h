#ifndef rl_error_h
#define rl_error_h

#include "runtime.h"

/* Error APIs */
char*    rl_err_name(rl_err_t e);
rl_err_t rl_error(rl_err_t e, const char* fn, const char* fmt, ...);
void     rl_fatal_err(rl_err_t e, const char* fn, const char* fmt, ...);

/* Error checking helpers. */
rl_err_t chk_argc(rl_err_t e, const char* fn, size_t x, size_t g, bool v);
rl_err_t chk_type(rl_err_t e, const char* fn, Type* x, Type* g);

#endif
