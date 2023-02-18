#ifndef data_sym_h
#define data_sym_h

#include "base/object.h"
#include "util/hash.h"

/* C types */
typedef enum SymFl SymFl;

enum SymFl {
  INTERNED=0b00000000001,
  LITERAL =0b00000000010
};

struct Sym {
  Obj obj;
  char *name;
  uint64 idno;

  /* invasive symbol table */
  Sym *left, *right;
};

/* globals */
extern Sym* SymbolTable;

/* API */
bool is_sym(Val x);
Sym* as_sym(Val x);
Val  mk_sym(Sym* s);

// predicates -----------------------------------------------------------------
bool is_literal(Sym* s);
bool is_interned(Sym* s);
bool is_defined(Sym* s);
bool is_bound(Sym* s);

// constructors ---------------------------------------------------------------
Sym *intern(char* name);
Val  symbol(char* name);
Val  keyword(char* name);

// environment utilities ------------------------------------------------------
Val lookup(Sym* s, Table* ns);
void assign(Sym* s, Val v, Table* ns);
Val define(char* name, Val val, Table* ns);

#endif
