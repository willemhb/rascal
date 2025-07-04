#ifndef rl_data_str_h
#define rl_data_str_h

/* User string type.

   Small strings are interned, making them the basis for fast symbol comparison. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define as_str(x)      ((Str*)as_obj(x))
#define is_interned(s) ((s)->flags == true)

// C types --------------------------------------------------------------------
struct Str {
  HEAD;

  char*  val;
  size_t count;
  hash_t hash;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Str* as_str_s(char* f, Expr x);
Str* mk_str(char* cs);

// initialization -------------------------------------------------------------
void toplevel_init_data_str(void);

#endif
