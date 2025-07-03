#ifndef rl_data_type_num_h
#define rl_data_type_num_h

/* Basic numeric type.

   It has a couple of internal representations,
   mostly for saving non-numeric data on the stack
   while marking it as something that won't be
   (or shouldn't be) tampered with. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Num    as_num_s(char* f, Expr x);
Num    as_num(Expr x);
Expr   tag_num(Num n);
uptr_t as_fix(Expr x);
Expr   tag_fix(uintptr_t i);
void*  as_ptr(Expr x);
Expr   tag_ptr(void* ptr);

// initialization -------------------------------------------------------------
void toplevel_init_data_type_num(void);

#endif
