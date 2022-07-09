#include <setjmp.h>
#include <stdarg.h>

#include "runtime/error.h"


// exports --------------------------------------------------------------------
static void verror(char *fmt, va_list va) {
  vfprintf( stderr, fmt, va );
  fputs( ".\n", stderr );
}

void rerror(char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  verror( fmt, va );
  va_end(va);
  longjmp(Toplevel, 1);
}

void require( bool test, char *fmt, ...) {
  if (!test) {
    va_list va;
    va_start(va, fmt);
    verror( fmt, va );
    va_end(va);
    longjmp(Toplevel, 1);
  }  
}
