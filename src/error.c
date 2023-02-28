#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "io.h"

// globals --------------------------------------------------------------------
#define ERR_BUF_SIZE 4096
error_t  Panicking;
value_t  Agitant;

char     ErrMsg[ERR_BUF_SIZE];

const char* ErrorNames[] = {
  [NO_ERROR]    = "no-error",
  [READ_ERROR]  = "read-error",
  [EVAL_ERROR]  = "eval-error",
  [APPLY_ERROR] = "apply-error"
};

// local helpers --------------------------------------------------------------
static void init_error(void) {
  Panicking = NO_ERROR;
  Agitant   = NUL;

  memset(ErrMsg, '\0', ERR_BUF_SIZE);
}

// API ------------------------------------------------------------------------
error_t raise_error(error_t err, value_t agitant, const char* fmt, ...) {
  if (Panicking)
    return Panicking;

  Panicking = err;
  Agitant   = agitant;

  if (err == READ_ERROR)
    read_error(agitant);

  va_list va; va_start(va, fmt); vsnprintf(ErrMsg, ERR_BUF_SIZE, fmt, va); va_end(va);

  return err;
}

error_t catch_error(value_t* agbuf) {
  error_t out = Panicking;

  if (out) {
    fprintf(stderr, "%s: %s.\n", ErrorNames[out], ErrMsg);

    if (agbuf)
      *agbuf = Agitant;
    
    init_error();
  }

  return out;
}

bool panicking(void) {
  return Panicking != NO_ERROR;
}

// initialization -------------------------------------------------------------
void error_init(void) {
  init_error();
}
