#include <stdarg.h>

#include "error.h"

/* API */
void panic_mode(const char* fmt, ...) {
  if (panicking())
    return;

  va_list va;
  printf("\n");
  va_start(va, fmt);
  vprintf(fmt, va);
  printf("\n");
  va_end(va);
  Vm.panic_mode = true;
}

bool panicking(void) {
  return Vm.panic_mode;
}

bool recover(void) {
  bool out = Vm.panic_mode;
  Vm.panic_mode = false;

  return out;
}
