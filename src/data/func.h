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
  List* ns;
  List* env;
} Chunk;

typedef union {
  NativeFn native;
  ReaderFn reader;
  Chunk*   user;
} FuncData;

// flags ----------------------------------------------------------------------
typedef enum FuncFl FuncFl;

enum FuncFl {
  NATIVE =0b00000001,
  READER =0b00000010,
  USER   =0b00000011,
  VARGS  =0b00000100,
  CLOSURE=0b00001000,
  TYPE   =0b00010000
};

// function object ------------------------------------------------------------
struct Func {
  Obj obj;
  Sym* name;
  Mtable* type; // if this function is a constructor
  GuardFn guard;
  FuncData func;
};

/* API */
bool  is_func(Val x);
Func* as_func(Val x);
Val   mk_func(Func* x);

// predicates -----------------------------------------------------------------
bool is_native(Func* f);
bool is_reader(Func* f);
bool is_user(Func* f);
bool is_vargs(Func* f);
bool is_closure(Func* f);

// metadata -------------------------------------------------------------------
usize func_argc(Func* f);
usize func_varc(Func* f);

// constructors ---------------------------------------------------------------
Func* native(char* name, usize argc, bool vargs, GuardFn guard, NativeFn func);
Func* reader(ReaderFn func);
Func* type(char* name, usize argc, bool vargs, GuardFn guard, NativeFn func, Mtable *mtable);


#endif
