#include <stdio.h>
#include <string.h>

#include "value.h"
#include "memory.h"
#include "number.h"

// globals --------------------------------------------------------------------
// symbol table ---------------------------------------------------------------
uword     SymbolCounter = 1;
symbol_t* SymbolTable = NULL;

extern object_t *LiveObjects;
value_t Unbound = UNBOUND;

// values ---------------------------------------------------------------------
// general --------------------------------------------------------------------
type_t type_of(value_t val) {
  switch (tag_of(val)) {
    case FIXNUMTAG: return FIXNUM;
    case NUL:       return UNIT;
    case BOOLTAG:   return BOOL;
    case NATIVETAG: return NATIVE;
    case OBJTAG:    return as_object(val)->type;
    default:        return REAL;
  }
}

char* type_name_of(value_t val) {
  return type_name_of_type(type_of(val));
}

char* type_name_of_type(type_t t) {
  static char* type_names[] = {
    [UNIT]   = "unit",
    [BOOL]   = "bool",
    [NATIVE] = "native",
    [REAL]   = "real",
    [FIXNUM] = "fixnum",
    [SYMBOL] = "symbol",
    [TUPLE]  = "tuple",
    [LIST]   = "list",
    [BINARY] = "binary"
  };

  return type_names[t];
}

bool is_real(value_t val) {
  return (val&QNAN) != QNAN;
}

bool is_fixnum(value_t val) {
  return type_of(val) == FIXNUM;
}

bool is_native(value_t val) {
  return type_of(val) == NATIVE;
}

bool is_unit(value_t val) {
  return val == NUL;
}

bool is_bool(value_t val) {
  return type_of(val) == BOOL;
}

bool is_object(value_t val) {
  return tag_of(val) == OBJTAG;
}

bool is_symbol(value_t val) {
  return type_of(val) == SYMBOL;
}

bool is_list(value_t val) {
  return type_of(val) == LIST;
}

bool is_binary(value_t val) {
  return type_of(val) == BINARY;
}

bool is_byte(value_t val) {
  return is_fixnum(val) && as_fixnum(val) <= UINT8_MAX;
}

bool is_function(value_t val) {
  return is_native(val);
}

bool is_number(value_t val) {
  type_t type = type_of(val);

  return type == REAL || type == FIXNUM;
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

real_t as_number(value_t val) {
  if (is_real(val))
    return as_real(val);

  return as_fixnum(val);
}

void* as_ptr(value_t val) {
  return (void*)val_of(val);
}

// object apis ----------------------------------------------------------------
static void init_object(object_t* object, type_t type, flags fl) {
  object->next  = LiveObjects;
  LiveObjects   = object;
  object->type  = type;
  object->flags = fl;
  object->black = false;
  object->gray  = true;
  object->hash  = 0;
}

// symbol ---------------------------------------------------------------------
static symbol_t** find_symbol(char* name) {
  symbol_t** node = &SymbolTable;

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
  self->bind  = Unbound;
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
    init_list(curr, args[i-1], last);
    last = curr--;
  }

  return tag_ptr(out, OBJTAG);
}

value_t nth_hd(list_t* xs, usize n) {
  assert(n < xs->len);

  while (n--)
    xs = xs->tail;

  return xs->head;
}

list_t* nth_tail(list_t* xs, usize n) {
  assert(n < xs->len);

  while (n--)
    xs = xs->tail;

  return xs;
}

// binary ---------------------------------------------------------------------
binary_t EmptyBinary = {
  .obj={
    .next =NULL,
    .hash =0,
    .flags=0,
    .type =BINARY,
    .gray =false,
    .black=true
  },
  .len=0
};

static binary_t* allocate_binary(usize n) {
  return allocate(sizeof(binary_t) + n * sizeof(ubyte));
}

static void init_binary(binary_t* bin, usize n, value_t* args) {
  init_object(&bin->obj, BINARY, 0);

  bin->len = n;

  for (usize i=0; i<n; i++)
    bin->array[i] = as_fixnum(args[i]);
}

value_t binary(usize n, value_t* args) {
  binary_t* bin;

  if (n == 0)
    bin = &EmptyBinary;

  else {
    bin = allocate_binary(n);
    init_binary(bin, n, args);
  }

  return tag_ptr(bin, OBJTAG);
}

// tuple ----------------------------------------------------------------------
tuple_t EmptyTuple = {
  .obj={
    .next=NULL,
    .hash=0,
    .flags=0,
    .type=TUPLE,
    .black=true,
    .gray=false
  },
  .len=0
};

static tuple_t* allocate_tuple(usize n) {
  assert(n > 0);

  return allocate(sizeof(tuple_t) + n * sizeof(value_t));
}

static void init_tuple(tuple_t* self, usize n, value_t* args) {
  assert(n > 0);
  assert(self != &EmptyTuple);

  init_object(&self->obj, TUPLE, 0);
  self->len = n;
  memcpy(self->slots, args, n*sizeof(value_t));
}

value_t tuple(usize n, value_t* args) {
  tuple_t* tup;

  if (n == 0)
    tup = &EmptyTuple;

  else {
    tup = allocate_tuple(n);
    init_tuple(tup, n, args);
  }

  return tag_ptr(tup, OBJTAG);
}
