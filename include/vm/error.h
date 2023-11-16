#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "vm/context.h"

/* Internal error handling utilities. */

#define rl_try                                                          \
  ErrFrame __err; int l__tr, l__ca;                                     \
  save_state(&__err); Ctx.err = &__err;                                 \
  if (!setjmp(__err.Cstate))                                            \
    for (l__tr=1; l__tr; l__tr=0, (void)(RlVm.err=__err.err))

#define rl_catch                                                \
  else                                                          \
    for (l__ca=1; l__ca; l__ca=0, restore_state(&__err))

void save_state(ErrFrame* err);
void restore_state(ErrFrame* err);

void   error(const char* fname, const char* fmt, ...);
void   syntax_error(List* form, const char* fmt, ...);
bool   require(bool test, const char* fname, const char* fmt, ...);
size_t argco(size_t expected, size_t got, const char* fname);
size_t vargco(size_t expected, size_t got, const char* fname);
Type*  argtype(Type* expected, Value got, const char* fname);
size_t argcos(size_t expected, size_t got, const char* fname, ...);
size_t vargcos(size_t expected, size_t got, const char* fname, ...);
Type*  argtypes(size_t expected, Value got, const char* fname, ...);

#endif
