#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "vm.h"
#include "runtime.h"

// external API
void initContext(Context* ctx) {
  (void)ctx;
}

void freeContext(Context* ctx) {
  (void)ctx;
}

void resetContext(Context* ctx) {
  ctx->panicking = false;
}

void panic(Vm* vm, Value cause, List* form, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vpanic(vm, cause, form, fmt, va);
  va_end(va);
}

void vpanic(Vm* vm, Value cause, List* form, const char* fmt, va_list va) {
  if (panicking(vm))
    return;

  fprintf(stderr, fmt, "error: ");
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");

  if (cause != NOTHING_VAL)
    
}

void recover(Vm* vm);
