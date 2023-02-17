#ifndef func_h
#define func_h

#include "base/object.h"

/* C types */
// function pointers ----------------------------------------------------------
typedef void (*GuardFn)(Func* callee, int nargs, Val* args);
typedef Val (*NativeFn)(int nargs, Val* args);
typedef Val (*ReaderFn)(Glyph g, Stream* s);

// user function --------------------------------------------------------------
typedef struct {
  Vec*  consts;
  Bin*  code;
  Cons* ns;
  Cons* env;
} Chunk;

// flags ----------------------------------------------------------------------
typedef enum FuncFl FuncFl;

enum FuncFl {
  NATIVE_FN=1,
  READER_FN=2,
  USER_FN  =3,
  VARGS_FN =4,
  CLOSED_FN=8,
};

// function object ------------------------------------------------------------
struct Func {
  Obj obj;
  Sym* name;
  GuardFn guard;

  union {
    NativeFn native;
    ReaderFn reader;
    Chunk    user;
  };
};

/* API */


#endif
