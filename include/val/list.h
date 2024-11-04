#ifndef rl_val_list_h
#define rl_val_list_h

#include "val/object.h"

/* Types, APIs, and globals supporting Rascal lists and pairs. */

/* C types. */
typedef enum Reference : uint8 {
  R_TABLE   = 1, // common table reference
  R_LOCAL   = 2, // local variable reference
  R_UPVAL_L = 3, // local upvalue reference (lexical)
  R_UPVAL_N = 4, // non-local upvalue reference (lexical)
  R_MODULE  = 5, // module variable reference
  R_GLOBAL  = 6, // global variable reference
  R_UPVAL_O = 7, // open upvalue (dynamic)
  R_UPVAL_C = 8, // closed upvalue (dynamic)
} Reference;

struct Cons {
  HEADER;
  uint8 proper;
  uint8 reftype;
  
  Val car;
  
  size32 arity;
  size32 offset;
  
  union {
    Val   cdr;
    Cons* tail;
  };
};

/* External APIs */
/* List API */
#define is_cons(x) has_type(x, T_CONS)
#define as_cons(x) ((Cons*)as_obj(x))

Cons* mk_cons(Val car, Val cdr);
Cons* mk_ref(Val key, Val val, Reference type, size32 arity, size32 offset);
Cons* mk_list2(Val hd, Cons* tl);
Cons* mk_listn(size32 n, Val* d);
Val   list_ref(Cons* xs, size32 n);

#define mk_list(x, y)                           \
  generic((y),                                  \
          Val*:mk_listn,                        \
          Cons*:mk_list2)(x, y)

#endif
