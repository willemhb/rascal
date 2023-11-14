#include "util/unicode.h"

#include "vm/memory.h"

#include "val/map.h"
#include "val/func.h"
#include "val/type.h"
#include "val/symbol.h"

/* Globals */
idno_t SymbolCounter = 0;
Symbol* Symbols      = NULL;
Symbol* Keywords     = NULL;

extern void trace_sym(void* obj);
extern void free_sym(void* obj);

INIT_OBJECT_TYPE(Symbol, NULL, trace_sym, free_sym, NULL);

/* external API */
static Symbol** find_in_symt(char* name, Symbol** root) {
  while (*root) {
    int o = strcmp(name, (*root)->name);

    if (o < 0)
      root = &(*root)->left;

    else if (o > 0)
      root = &(*root)->right;

    else
      break;
  }

  return root;
}

Symbol* new_sym(char* name, flags_t fl) {
  size_t l = strlen(name);
  Symbol* out = new_obj(&SymbolType, fl, 0, 0);
  out->left = out->right = NULL;
  out->name = duplicates(NULL, name, l);
  out->idno = ++SymbolCounter;
  out->form = NULL;

  return out;
}

Symbol* mk_sym(char* name, bool gensym) {
  bool literalp = *name == ':';
  flags_t flags = literalp*LITERAL | !(gensym)*INTERNED;

  if (literalp)
    name++;

  assert(*name != '\0');

  if (gensym)
    return new_sym(name, flags);

  Symbol** buf = find_in_symt(name, literalp ? &Keywords : &Symbols);

  if (*buf == NULL)
    *buf = new_sym(name, flags);

  return *buf;
}

Symbol* intern_sym(Symbol* sym) {
  Symbol** buf = find_in_symt(sym->name, is_literal(sym) ? &Keywords : &Symbols);

  assert(*buf == NULL);

  *buf      = sym;
  sym->idno = ++SymbolCounter;
  return sym;
}
