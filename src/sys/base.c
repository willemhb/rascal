/**
 *
 * Common declarations, APIs, and helpers for Rascal runtime system.
 *
 **/

// headers --------------------------------------------------------------------
#include <string.h>

#include "sys/base.h"
#include "sys/error.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
// vm helpers -----------------------------------------------------------------
void reset_vals(void) {
  memset(Vm.vals, 0, N_VALS * sizeof(Expr));
  Vm.sp = 0;
  Vm.bp = 0;
}

void reset_frames(void) {
  memset(Vm.frames, 0, N_FRAMES * sizeof(Expr));
  Vm.fp = 0;
}

void reset_vm(void) {
  Vm.upvs = NULL;
  Vm.pc   = NULL;
  Vm.fn   = NULL;

  reset_vals();
  reset_frames();
}

// reader helpers -------------------------------------------------------------
void reset_token(void) {
  memset(Token, 0, BUFFER_SIZE);
  TOff = 0;
}

size_t add_to_token(char c) {
  if ( TOff < BUFFER_MAX )
    Token[TOff++] = c;

  else
    runtime_error("maximum token length exceeded");

  return TOff;
}

// initialization -------------------------------------------------------------
