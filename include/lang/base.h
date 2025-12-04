#ifndef rl_lang_base_h
#define rl_lang_base_h

/* Globals and common APIs for the Rascal interpreter. */
// headers --------------------------------------------------------------------
#include <stdarg.h>

#include "common.h"

#include "sys/opcode.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
// Names for special characters (\0, \n, \t, etc)
extern char* CharNames[128];

// function prototypes --------------------------------------------------------
// Core expression APIs -------------------------------------------------------
hash_t hash_exp(Expr x);
bool egal_exps(Expr x, Expr y);

// Core API helpers -----------------------------------------------------------
bool egal_traverse(void* ox, void* oy);
bool egal_exp_arrays(size_t xn, Expr* xs, size_t yn, Expr* ys);

// local environment utilities ------------------------------------------------
Expr upval_ref(Fun* fun, int i);
void upval_set(Fun* fun, int i, Expr x);

// miscellaneous utilities ----------------------------------------------------
bool is_falsey(Expr x);
bool is_literal(Expr x);

// initialization -------------------------------------------------------------

#endif
