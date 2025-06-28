#ifndef rl_data_expr_h
#define rl_data_expr_h

/* Core type information, globals, and toplevel expression APIs. */

// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------
// Expression tags
#define QNAN   0x7ffc000000000000ul

#define NONE_T  0x7ffd000000000000ul
#define NUL_T   0x7ffe000000000000ul
#define EOS_T   0x7fff000000000000ul
#define BOOL_T  0xfffc000000000000ul
#define GLYPH_T 0xfffd000000000000ul
#define FIX_T   0xfffe000000000000ul
#define OBJ_T   0xffff000000000000ul

// masks
#define XTMSK  0xffff000000000000ul
#define XVMSK  0x0000fffffffffffful

// utility macros
#define exp_tag(x)     ((x) & XTMSK)
#define exp_val(x)     ((x) & XVMSK)

// limits
#define NUM_TYPES (EXP_NUM+1)

// C types --------------------------------------------------------------------
// Expression type codes
typedef enum {
  EXP_NONE,
  EXP_NUL,
  EXP_EOS,
  EXP_BOOL,
  EXP_GLYPH,
  EXP_CHUNK,
  EXP_ALIST,
  EXP_BUF16,
  EXP_REF,
  EXP_UPV,
  EXP_ENV,
  EXP_PORT,
  EXP_FUN,
  EXP_SYM,
  EXP_STR,
  EXP_LIST,
  EXP_NUM
} ExpType;

typedef struct {
  ExpType type;
  char*   name;
  Sym*    repr;       // rascal representation of the type (for now just a keyword)
  size_t  obsize;
  PrintFn print_fn;
  HashFn  hash_fn;
  EgalFn  egal_fn;
  CloneFn clone_fn;
  TraceFn trace_fn;
  FreeFn  free_fn;
} ExpTypeInfo;

// function prototypes --------------------------------------------------------
ExpType      exp_type(Expr x);
bool         has_type(Expr x, ExpType t);
ExpTypeInfo* exp_info(Expr x);
hash_t       hash_exp(Expr x);
bool         egal_exps(Expr x, Expr y);
void         mark_exp(Expr x);

// globals --------------------------------------------------------------------
extern ExpTypeInfo Types[];

#endif
