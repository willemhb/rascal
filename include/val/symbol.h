#ifndef rl_val_symbol_h
#define rl_val_symbol_h

#include "val/object.h"

/* declarations & APIs for Symbol type.

   Symbols are first class identifiers.

   They are typically interned in a global Symbol table, stored as an invasive
   tree in the Symbol's left and right pointers.

   Keywords are symbols whose name is prefixed with ':'. Keywords are used as flags,
   options, */

/* C types */
typedef size_t (*SpclForm)(List form);

typedef enum {
  INTERNED=0x01u,
  LITERAL =0x02u,
} SymFl;

struct Symbol {
  HEADER;
  Symbol*  left, * right;
  char*    name;
  idno_t   idno;
  SpclForm form;
};

/* globals */
// symbol tables
extern struct Type SymbolType;
extern Symbol* Symbols, * Keywords;

#define is_sym(x)      has_type(x, &SymbolType)
#define as_sym(x)      as(Symbol*, untag48, x)
#define is_literal(x)  get_fl(x, LITERAL)
#define is_interned(x) get_fl(x, INTERNED)

/* external API */
Symbol* new_sym(char* name, flags_t fl);
Symbol* mk_sym(char* name, bool gensym);
Symbol* intern_sym(Symbol* sym);           // add static symbol to symbol table

#endif
