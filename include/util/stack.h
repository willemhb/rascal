#ifndef rl_util_stack_h
#define rl_util_stack_h

#include "runtime.h"

/* Definitions and APIs for stack types. */
#define STACK_TYPE(T, X)                        \
  typedef struct {                              \
    X* base;                                    \
    X* end;                                     \
    X* next;                                    \
  } T

STACK_TYPE(ValStack, Val);
STACK_TYPE(IfStack, IFrame);

/* ValStack API */
#define STACK_OP(T, op, x, ...)                                 \
  generic((x),                                                  \
          T*:op##_u,                                            \
          const char*:op##_s)((x) __VA_OPT__(,) __VA_ARGS__)

#define vs_push(x, ...)                                         \
  STACK_OP(ValStack, vs_push, (x) __VA_OPT__(,) __VA_ARGS__)

#define vs_write(x, ...)                                        \
  STACK_OP(ValStack, vs_write, (x) __VA_OPT__(,) __VA_ARGS__)

#define vs_pushn(x, ...)                                        \
  STACK_OP(ValStack, vs_pushn, (x) __VA_OPT__(,) __VA_ARGS__)

#define vs_pushv(x, ...)                                        \
  STACK_OP(ValStack, vs_pushv, (x) __VA_OPT__(,) __VA_ARGS__)

#define vs_pop(x, ...)                                      \
  STACK_OP(ValStack, vs_pop, (x) __VA_OPT__(,) __VA_ARGS__)

#define vs_popn(x, ...)                                         \
  STACK_OP(ValStack, vs_popn, (x) __VA_OPT__(,) __VA_ARGS__)

#define vs_popr(x, ...)                                         \
  STACK_OP(ValStack, vs_popr, (x) __VA_OPT__(,) __VA_ARGS__)

void     init_vs(ValStack* s, Val* b, Val* e);
void     reset_vs(ValStack* s);
void     trace_vs(ValStack* s);
Val*     vs_push_u(ValStack* s, Val x);
rl_err_t vs_push_s(const char* fn, Val** r, ValStack* s, Val x);
Val*     vs_write_u(ValStack* s, Val* x, size_t n);
rl_err_t vs_write_s(const char* fn, Val** r, ValStack* s, Val* x, size_t n);
Val*     vs_pushn_u(ValStack* s, size_t n, ...);
rl_err_t vs_pushn_s(const char* fn, ValStack* s, Val** r, size_t n, ...);
Val*     vs_pushv_u(ValStack* s, size_t n, va_list va);
rl_err_t vs_pushv_s(const char* fn, Val** r, ValStack* s, size_t n, va_list va);
Val      vs_pop_u(ValStack* s);
rl_err_t vs_pop_s(const char* fn, Val* r, ValStack* s);
Val      vs_popn_u(ValStack* s, size_t n, bool e);
rl_err_t vs_popn_s(const char* fn, Val* r, ValStack* s, size_t n, bool e);
Val      vs_popr_u(ValStack* s, size_t n);
rl_err_t vs_popr_s(const char* fn, Val* r, ValStack* s, size_t n);

/* IfStack API */
#define ifs_push(x, ...)                                        \
  STACK_OP(IfStack, ifs_push, (x) __VA_OPT__(,) __VA_ARGS__)

#define ifs_write(x, ...)                                       \
  STACK_OP(IfStack, ifs_write, (x) __VA_OPT__(,) __VA_ARGS__)

#define ifs_pop(x, ...)                                     \
  STACK_OP(IfStack, ifs_pop, (x) __VA_OPT__(,) __VA_ARGS__)

#define ifs_popn(x, ...)                                        \
  STACK_OP(IfStack, ifs_popn, (x) __VA_OPT__(,) __VA_ARGS__)

void     init_ifs(IfStack* s, IFrame* b, IFrame* e);
void     reset_ifs(IfStack* s);
void     trace_ifs(IfStack* s);
IFrame*  ifs_push_u(IfStack* s, IFrame f);
rl_err_t ifs_push_s(const char* fn, IFrame** r, IfStack* s, IFrame f);
IFrame*  ifs_write_u(IfStack* s, IFrame* f, size_t n);
rl_err_t ifs_write_s(const char* fn, IFrame** r, IfStack* s, IFrame* f, size_t n);
IFrame   ifs_pop_u(IfStack* s);
rl_err_t ifs_pop_s(const char* fn, IFrame* r, IfStack* s);
IFrame   ifs_popn_u(IfStack* s, size_t n, bool e);
rl_err_t ifs_popn_s(const char* fn, IFrame* r, IfStack* s, size_t n, bool e);

#undef STACK_TYPE

#endif
