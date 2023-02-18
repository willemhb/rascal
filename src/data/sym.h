#ifndef data_sym_h
#define data_sym_h

#include "base/object.h"
#include "util/hash.h"

/* C types */
typedef enum SymFl SymFl;

enum SymFl {
  INTERNED=0b00000000001,
  LITERAL =0b00000000010,
  DEFINED =0b00000000100,
  BOUND   =0b00000001000
};

struct Sym {
  Obj obj;
  char *name;
  Val bind;

  /* invasive symbol table */
  Sym *left, *right;
};

/* globals */
extern Sym* SymbolTable;

/* API */
bool is_sym(Val x);
Sym* as_sym(Val x);
Val tag_sym(Sym* s);

// predicates -----------------------------------------------------------------
bool is_literal(Sym* s);
bool is_interned(Sym* s);
bool is_defined(Sym* s);
bool is_bound(Sym* s);

Sym* intern(char* name);
Val lookup(Sym* s);
Val assign(Sym* s);
Val define(char* name, Val val, bool idem);

#endif
