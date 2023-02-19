#ifndef types_object_h
#define types_object_h

#include "types/value.h"
#include "types/type.h"

/* C types */
// object unitype -------------------------------------------------------------
struct Obj {
  Obj   *next;
  Type   type;
  uint   flags  : 29;
  uint   nofree :  1;
  uint   black  :  1;
  uint   gray   :  1;

  ubyte space[0];
};

// symbol type ----------------------------------------------------------------
typedef uint64 uhash;
typedef uint64 uidno;

typedef enum SymFl {
  INTERNED=0x00000001,
  LITERAL =0x00000002
} SymFl;

struct Sym {
  Obj   obj;
  char *name;

  uhash hash;
  uidno idno;

  Val constant; // special toplevel constant

  Sym *left, *right; // invasive symbol table
};

// list type ------------------------------------------------------------------
struct List {
  union {
    Obj obj;

    struct {
      Obj* next;
      Type type;
      uint len    : 29;
      uint nofree :  1;
      uint black  :  1;
      uint gray   :  1;
    };
  };

  Val   head;
  List* tail;
};

// func type ------------------------------------------------------------------
typedef Val (*NativeFn)(Val* pos, List* opt, Table* kw, List* va);

typedef struct {
  Table* consts;
  Bin*   code;
  List*  ns;
  List*  env;
} Chunk;

typedef enum FuncFl {
  NATIVE=0x10000000,
  USER  =0x08000000,
} FuncFl;

typedef union {
  NativeFn native;
  Chunk*   user;
} FuncData;

struct Func {
  union {
    Obj obj;
    struct {
      Obj* _next;
      Type _type;
      uint arity   : 27;
      uint user    :  1;
      uint native  :  1;
      uint _nofree :  1;
      uint _black  :  1;
      uint _gray   :  1;
    };
  };

  Sym* name;
  Mtable* type; // if this function is a constructor, this is the type it constructs
  FuncData func;
};

/* globals */
extern Sym*  SymbolTable;
extern List EmptyList;

/* API */
// common api ------------------------------------------------------------------
void init_obj(Obj* self, Type type);
void mark_obj(Obj* self);
void destruct_obj(Obj* self);
bool has_flag(Obj* o, flags fl);
bool set_flag(Obj* o, flags fl);
bool clr_flag(Obj* o, flags fl);

// symbol api -----------------------------------------------------------------
Sym *intern(char* name);
Val  keyword(Val val);
Val  symbol(Val val);
Val  gensym(Val val);
void set_const(Sym* sym, Val val);
bool is_gensym(Sym* sym);
bool is_literal(Sym* sym);
bool is_keyword(Sym* sym);

// list api -------------------------------------------------------------------
List* cons(Val head, List* tail);
List* list(uint n, ...);

List* list_assc(List* ls, Val k);
Val   list_nth(List* ls, uint n);

// func api ------------------------------------------------------------------
Func* new_func(bool native, uint arity, Val name, Mtable* type, void* func);
bool  is_type(Func* f);
bool  is_closure(Func* f);

// initialization ------------------------------------------------------------
void object_init(void);

#endif
