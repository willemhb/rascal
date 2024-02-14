#ifndef rl_val_symbol_h
#define rl_val_symbol_h

#include "val/object.h"

/* declarations & APIs for Symbol type.

   Symbols are first class identifiers.

   Unlike in traditional lisps, Symbols are not interned directly. However, its named components are, allowing for constant-time comparison.

   Symbols have a name part and optional module part, separated by a '/'.

   If the module part is ommitted, it is treated as a lexical reference, meaning
   that a matching name is searched first in the function scope, then in the module
   scope, and finally in the global scope.

   Keywords are symbols whose name is prefixed with ':'. Keywords are used as flags,
   options, and syntactic markers. */

/* C types */

struct Symbol {
  HEADER;
  String* module;
  String* name;
  idno_t  idno;    // Gensym counter. If 0, this is not a gensym.
  bool    literal; // Indicates whether this symbol should be treated as 
};

/* globals */
// symbol tables
extern struct Type SymbolType;

#define is_sym(x) has_type(x, &SymbolType)
#define as_sym(x) as(Symbol*, untag48, x)

#define is_gensym(x)  (!!(as_sym(x)->idno))
#define is_literal(x) (as_sym(x)->literal)

/* external API */
Symbol* mk_sym(String* name, String* module, bool gensym);

#endif
