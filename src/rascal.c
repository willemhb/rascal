#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Magic numbers
#define BUFFER_SIZE 2048

// Types
// Token types
typedef enum {
  TOKEN_READY,
  TOKEN_SYM,
  TOKEN_NUL,
  TOKEN_NUM,
  TOKEN_END
} Token;

// Expression types
typedef enum {
    EXP_NUL,
    EXP_SYM,
    EXP_NUM
} ExpType;

typedef enum {
  OKAY,
  PROBLEM
} Status;

typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double    Num;
typedef struct    Obj Obj;
typedef struct    Sym Sym;

typedef union {
  Expr expr;
  Num  num;
  Obj* obj;
  Nul  nul;
} Val;

#define HEAD                                    \
  Obj* next;                                    \
  ExpType type;                                 \
  bool marked

struct Obj {
  HEAD;
};

struct Sym {
  HEAD;

  char* val;
};

// Globals
const char* ExpTypeName[] = {
  [EXP_NUL] = "nul",
  [EXP_SYM] = "sym",
  [EXP_NUM] = "num"
};

const size_t ExpTypeObjSize[] = {
  [EXP_NUL] = 0,
  [EXP_SYM] = sizeof(Sym),
  [EXP_NUM] = 0
};

// Expression tags
#define QNAN  0x7ffc000000000000ul

#define XTMSK 0xffff000000000000ul
#define XVMSK 0x0000fffffffffffful

#define OBJ   0xfffc000000000000ul
#define NUL   0xffff000000000000ul

// miscellaneous helpers
#define error_message(fmt, ...) fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

// forward declarations
ExpType expr_type(Expr x);
void   run_gc(void);
Status read_expr(Expr* r);
Status eval_expr(Expr* r, Expr x);
void print_expr(Expr x);
void rl_repl(void);

// expression APIs
Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

Obj* as_obj(Expr x) {
  return (Obj*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ;
}

#define as_sym(x) ((Sym*)as_obj(x))

// implmentations
ExpType expr_type(Expr x) {
  ExpType t;
  
  switch ( x & XTMSK ) {
    case NUL: t = EXP_NUL;         break;
    case OBJ: t = as_obj(x)->type; break;
    default:  t = EXP_NUM;         break;
  }

  return t;
}

// read helpers
void skip_space(FILE* f) {
  while ( !feof(f) ) {
    int c = fgetc(f);

    if ( isspace(c) )
      continue;

    else if ( c == EOF )
      break;

    else {
      ungetc(c, f);
      break;
    }
  }
}


Status read_atom(Expr* r) {

  return s;
}


Token get_token(FILE* f, Expr* r, char buffer[BUFFER_SIZE]) {
  skip_space(f);
  
  Token t;
  
  if ( feof(f) )
    t = TOKEN_END;

  else {
    
  }
}

Status read_expr(Expr* r) {
  Status s = OKAY;
  char buffer[BUFFER_SIZE];

  skip_space(stdin);

  if ( )

  return s;
}

Status eval_expr(Expr* r, Expr x) {
  Expr v;
  Status s = OKAY;
  
  switch ( expr_type(x) ) {
    default: v = x; break;
  }

  *r = v;

  return s;
}

void print_expr(Expr x) {
  switch ( expr_type(x) ) {
    case EXP_NUL: printf("nul"); break;
    case EXP_NUM: printf("%.2g", as_num(x)); break;
    default:      printf("<%s>", ExpTypeName[expr_type(x)]); break;
  }
}

// entry point
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  rl_repl();

  return 0;
}
