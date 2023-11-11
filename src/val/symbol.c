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

Symbol SymbolName = {
  .obj={
    .next =NULL,
    .hash =0,
    .type =&SymbolType,
    .meta =&EmptyDict,
    .flags=0,
    .memfl=NOSWEEP|NOFREE|GRAY,
  },
  .left =NULL,
  .right=NULL,
  .name ="Symbol",
  .idno =0,
  .form =NULL
};

Func SymbolCTor = {
  .obj={
    .next =NULL,
    .hash =0,
    .type =&FuncType,
    .meta =&EmptyDict,
    .flags=0,
    .memfl=NOSWEEP|NOFREE|GRAY,
  },
  .name=&SymbolName,
  .func=NULL,
};

extern void trace_sym(Obj* slf);
extern void free_sym(Obj* slf);

Vtable SymbolVtable = {
  .vsize    =sizeof(Symbol*),
  .osize    =sizeof(Symbol),
  .tag      =OBJ_TAG,
  .size     =NULL,
  .trace    =trace_sym,
  .finalize =free_sym,
};

Type SymbolType = {
  .obj={
    .next =NULL,
    .hash =0,
    .type =&TypeType,
    .meta =&EmptyDict,
    .flags=DATA_KIND,
    .memfl=NOSWEEP|NOFREE|GRAY,
  },
  .idno   =0,
  .parent =&TermType,
  .name   =&SymbolName,
  .ctor   =&SymbolCTor,
  .members=NULL,
  .vtable =&SymbolVtable,
};

/* external API */
Symbol* as_sym(Value x) {
  return (Symbol*)untag_48(x);
}

bool val_is_sym(Value x) {
  return type_of(x) == &SymbolType;
}

bool obj_is_sym(Obj* obj) {
  return obj && obj->type == &SymbolType;
}
bool is_literal_sym(Symbol* s) {
  return get_obj_fl((Obj*)s, LITERAL);
}

bool is_interned_sym(Symbol* s) {
  return get_obj_fl((Obj*)s, INTERNED);
}

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
  Symbol* out = (Symbol*)new_obj(&SymbolType, fl, 0, 0);
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
  Symbol** buf = find_in_symt(sym->name, is_literal_sym(sym) ? &Keywords : &Symbols);

  assert(*buf == NULL);

  *buf      = sym;
  sym->idno = ++SymbolCounter;
  return sym;
}
