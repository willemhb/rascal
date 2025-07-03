/* DESCRIPTION */
// headers --------------------------------------------------------------------
#include "data/types/num.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_num(Port* ios, Expr x);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_num(Port* ios, Expr x) {
  pprintf(ios, "%g", as_num(x));
}

// external -------------------------------------------------------------------
Num as_num_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_NUM, "%s wanted type num, got %s", f, Types[t].name);
  return as_num(x);
}

Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

// shortcut for tagging pointers and small integers safely
uintptr_t as_fix(Expr x) {
  return x & XVMSK;
}

Expr tag_fix(uintptr_t i) {
  return ( i & XVMSK) | FIX_T;
}

void* as_ptr(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_ptr(void* ptr) {
  return ((uintptr_t)ptr) | FIX_T;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_num(void) {
  Types[EXP_NUM] = (ExpTypeInfo) {
    .type     = EXP_NUM,
    .name     = "num",
    .obsize   = 0,
    .print_fn = print_num
  };
}
