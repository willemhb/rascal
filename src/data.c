#include "data.h"
#include "runtime.h"
#include "lang.h"


// Globals
ExpTypeInfo Types[] = {
  [EXP_NUL] = {
    .type   = EXP_NUL,
    .name   = "nul",
    .obsize = 0
  },

  [EXP_EOS] = {
    .type   = EXP_EOS,
    .name   = "eos",
    .obsize = 0
  },

  [EXP_SYM] = {
    .type   = EXP_SYM,
    .name   = "sym",
    .obsize = sizeof(Sym)
  },

  [EXP_LIST] = {
    .type    = EXP_LIST,
    .name    = "list",
    .obsize  = sizeof(List)
  },

  [EXP_NUM]  = {
    .type    = EXP_NUM,
    .name    = "num",
    .obsize  = 0
  }
};
