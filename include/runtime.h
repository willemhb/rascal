#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "common.h"
#include "value.h"

// C types
struct Context {
  bool panicking;
};

// external API
void initContext(Context* ctx);
void freeContext(Context* ctx);
void resetContext(Context* ctx);

void panic(Vm* vm, List* form, Value cause, const char* fmt, ...);
void vpanic(Vm* vm, List* form, Value cause, const char* fmt, va_list va);
void recover(Vm* vm);

#endif
