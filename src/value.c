#include <string.h>

#include "value.h"
#include "memory.h"
#include "number.h"

// globals --------------------------------------------------------------------
// symbol table ---------------------------------------------------------------
uword     SymbolCounter = 1;
symbol_t* SymbolTable = NULL;

extern object_t *LiveObjects;

// values ---------------------------------------------------------------------
// general --------------------------------------------------------------------
type_t type_of(value_t val) {
  switch (tag_of(val)) {
    case NUL:       return UNIT;
    case NATIVETAG: return NATIVE;
    case OBJTAG:    return as_object(val)->type;
    default:        return REAL;
  }
}

char* type_name_of(value_t val) {
  static char* type_names[] = {
    [UNIT]   = "unit",
    [NATIVE] = "native",
    [REAL]   = "real",
    [SYMBOL] = "symbol",
    [LIST]   = "list"
  };

  return type_names[type_of(val)];
}

bool is_real(value_t val) {
  return (val&QNAN) != QNAN;
}

bool is_object(value_t val) {
  return tag_of(val) == OBJTAG;
}

value_t tag_ptr(void* p, uword t) {
  return tag_word((uword)p, t);
}

value_t tag_word(uword w, uword t) {
  return (w&VAL_MASK) | t;
}

value_t tag_dbl(double dbl) {
  return dtow(dbl);
}

real_t as_real(value_t val) {
  return wtod(val);
}

uword as_word(value_t val) {
  return val_of(val);
}

void* as_ptr(value_t val) {
  return (void*)val_of(val);
}

// object apis ----------------------------------------------------------------
static void init_object(object_t *object, type_t type, flags fl) {
  object->next  = LiveObjects;
  LiveObjects   = object;
  object->type  = type;
  object->flags = fl;
  object->black = false;
  object->gray  = true;
  object->hash  = 0;
}

// symbol ---------------------------------------------------------------------
static symbol_t **find_symbol(char* name) {
  symbol_t **node = &SymbolTable;

  while (*node) {
    int o = strcmp(name, (*node)->name);

    if (o < 0)
      node = &(*node)->left;

    else if (o > 0)
      node = &(*node)->right;

    else
      break;
  }

  return node;
}

static void init_symbol(symbol_t *self, char* name) {
  init_object(&self->obj, SYMBOL, 0);
  self->left  = NULL;
  self->right = NULL;
  self->idno  = SymbolCounter++;
  self->bind  = NUL;
  self->name  = strdup(name);
}

static symbol_t *new_symbol(char* name) {
  symbol_t *sym = allocate(sizeof(symbol_t));

  init_symbol(sym, name);

  return sym;
}

value_t symbol(char* name) {
  symbol_t **node = find_symbol(name);

  if (*node == NULL)
    *node = new_symbol(name);

  return tag_ptr(*node, OBJTAG);
}

// list -----------------------------------------------------------------------
list_t EmptyList = {
  .obj={
    .next =NULL,
    .type =LIST,
    .hash =0,
    .flags=0,
    .black=true,
    .gray =false
  },
  .len=0,
  .head=NUL,
  .tail=&EmptyList
};

static void init_list(list_t* self, value_t head, list_t* tail) {
  init_object(&self->obj, LIST, 0);

  self->head = head;
  self->tail = tail;
  self->len  = 1 + tail->len;
}

value_t cons(value_t head, list_t* tail) {
  list_t* out = allocate(sizeof(list_t));
  init_list(out, head, tail);
  return tag_ptr(out, OBJTAG);
}

value_t list(usize n, value_t* args) {
  if (n == 0)
    return NUL;

  if (n == 1)
    return cons(args[0], &EmptyList);

  list_t* out  = allocate(n * sizeof(list_t));
  list_t* curr = &out[n-1], *last = &EmptyList;

  for (usize i=n; i>0; i--) {
    init_list(curr, args[n-1], last);
    last = curr--;
  }

  return tag_ptr(out, OBJTAG);
}
