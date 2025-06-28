/* Core type information, globals, and toplevel expression APIs. */
// headers --------------------------------------------------------------------
#include "data/expr.h"
#include "data/obj.h"

#include "util/hashing.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
ExpTypeInfo Types[NUM_TYPES];

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
ExpType exp_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NONE_T  : t = EXP_NONE;      break;
    case NUL_T   : t = EXP_NUL;       break;
    case EOS_T   : t = EXP_EOS;       break;
    case BOOL_T  : t = EXP_BOOL;      break;
    case GLYPH_T : t = EXP_GLYPH;     break;
    case OBJ_T   : t = head(x)->type; break;
    default      : t = EXP_NUM;       break;
  }

  return t;
}

bool has_type(Expr x, ExpType t) {
  return exp_type(x) == t;
}

ExpTypeInfo* exp_info(Expr x) {
  return &Types[exp_type(x)];
}

hash_t hash_exp(Expr x) {
  hash_t out;
  ExpTypeInfo* info = exp_info(x);

  if ( info->hash_fn )
    out = info->hash_fn(x);

  else
    out = hash_word(x);

  return out;
}

bool egal_exps(Expr x, Expr y) {
  bool out;
  
  if ( x == y )
    out = true;

  else {
    ExpType tx = exp_type(x), ty = exp_type(y);

    if ( tx != ty )
      out = false;

    else {
      EgalFn fn = Types[tx].egal_fn;
      out       = fn ? fn(x, y) : false;
    }
  }

  return out;
}

void mark_exp(Expr x) {
  if ( exp_tag(x) == OBJ_T )
    mark_obj(as_obj(x));
}

// initialization -------------------------------------------------------------
