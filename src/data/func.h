#ifndef func_h
#define func_h

#include "base/object.h"

/* C types */
// function pointers ----------------------------------------------------------
typedef Val (*NativeFn)(int nargs, Val* args, Table* opts);
typedef Val (*ReaderFn)(Stream* s, Glyph g);

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
  VOPTS  =0b00001000,
  CLOSURE=0b00010000
};

// function object ------------------------------------------------------------
struct Func {
  Obj obj;
  int argc;
  int varc;
  Sym* name;
  Mtable* type; // if this function is a constructor
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
bool is_vopts(Func* f);
bool is_closure(Func* f);
bool is_type(Func* f);

// constructors ---------------------------------------------------------------
Func* native(char* name, int argc, int fl, NativeFn func);
Func* reader(ReaderFn func);
Func* type(char* name, int argc, int fl, NativeFn func, Mtable *mtable);

// miscellaneous utilities ----------------------------------------------------
void defnative(char* name, int argc, int fl, NativeFn func, ...);
void deftype(char* name, int argc, int fl, NativeFn func, Mtable *mtable, ...);

#endif
