#include "util/unicode.h"
#include "util/hashing.h"

#include "vm/memory.h"

#include "val/table.h"
#include "val/func.h"
#include "val/type.h"
#include "val/symbol.h"

/* Globals */
idno_t SymbolCounter = 0;
Symbol* Symbols      = NULL;
Symbol* Keywords     = NULL;

void trace_sym(void* obj) {
  Symbol* sym = obj;

  mark(sym->left);
  mark(sym->right);
}

void finalize_sym(void* obj) {
  Symbol* sym = obj;

  if (sym != NULL)
    deallocate(NULL, sym->name, 0);
}

hash_t hash_sym(Value x) {
  Symbol* sym = as_sym(x);
  hash_t hbase = hash_string(sym->name);
  hash_t hidno = hash_word(sym->idno);
  hash_t final = mix_hashes(hidno, hbase);

  return final;
}

int order_syms(Value x, Value y) {
  Symbol* symx = as_sym(x);
  Symbol* symy = as_sym(y);
  int out;

  // keywords always precede identifiers
  if (is_literal(symx) && !is_literal(symy))
    out = -1;

  else if (is_literal(symy))
    out = 1;

  else {
    out = strcmp(symx->name, symy->name);

    if (out == 0)
      out = 0 - (symx->idno < symy->idno) + (symx->idno > symy->idno);
  }

  return out;
}

INIT_OBJECT_TYPE(Symbol,
                 .tracefn=trace_sym,
                 .finalizefn=finalize_sym,
                 .hashfn=hash_sym,
                 .ordfn=order_syms);

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
