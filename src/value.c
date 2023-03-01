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

// dispatch tables ------------------------------------------------------------
usize size_of_tuple(void* ptr);
usize size_of_stencil(void* ptr);
usize size_of_binary(void* ptr);

usize (*SizeOf[NUM_TYPES])(void* ptr) = {
  [TUPLE]   = size_of_tuple,
  [STENCIL] = size_of_stencil,
  [BINARY]  = size_of_binary
};

usize BaseSize[NUM_TYPES] = {
  [UNIT]    = sizeof(NUL),
  [BOOL]    = sizeof(bool_t),
  [NATIVE]  = sizeof(native_t),
  [REAL]    = sizeof(real_t),
  [FIXNUM]  = sizeof(fixnum_t),
  [SYMBOL]  = sizeof(symbol_t),
  [TUPLE]   = sizeof(tuple_t),
  [LIST]    = sizeof(list_t),
  [VECTOR]  = sizeof(vector_t),
  [BINARY]  = sizeof(binary_t),
  [STENCIL] = sizeof(stencil_t)
};

usize size_of_tuple(void* ptr) {
  tuple_t* tup = ptr;

  return sizeof(tuple_t) + tup->len * sizeof(value_t);
}

usize size_of_stencil(void* ptr) {
  stencil_t* st = ptr;

  return sizeof(stencil_t) + stencil_len(st) * sizeof(value_t);
}

usize size_of_binary(void* ptr) {
  binary_t* bin = ptr;

  return sizeof(binary_t) + bin->len;
}

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

usize size_of_type(type_t type) {
  return BaseSize[type];
}

usize size_of_val(value_t val) {
  type_t t = type_of(val);

  if (SizeOf[t])
    return SizeOf[t](as_object(val));

  return size_of_type(t);
}

usize size_of_obj(void* ptr) {
  object_t* obj = ptr;

  if (SizeOf[obj->type])
    return SizeOf[obj->type](obj);

  return size_of_type(obj->type);
}

char* type_name_of(value_t val) {
  return type_name_of_type(type_of(val));
}

char* type_name_of_type(type_t t) {
  static char* type_names[] = {
    [NONE]    = "none",
    [UNIT]    = "unit",
    [BOOL]    = "bool",
    [NATIVE]  = "native",
    [REAL]    = "real",
    [FIXNUM]  = "fixnum",
    [SYMBOL]  = "symbol",
    [TUPLE]   = "tuple",
    [LIST]    = "list",
    [VECTOR]  = "vector",
    [BINARY]  = "binary",
    [STENCIL] = "stencil",
    [ANY]     = "any"
  };

  return type_names[t];
}

bool has_type(value_t val, type_t type) {
  switch (type) {
    case NONE: return false;
    case ANY:  return true;
    case UNIT: return val == NUL;
    default:   return type_of(val) == type;
  }
}

bool is_object(value_t val) {
  return tag_of(val) == OBJTAG;
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

bool has_flag(void* ptr, flags fl) {
  assert(ptr && (((uword)ptr) & 7) == 0);

  object_t* obj = ptr;

  return !!(obj->flags&fl);
}

bool set_flag(void* ptr, flags fl) {
  assert(ptr && (((uword)ptr) & 7) == 0);

  bool out = !has_flag(ptr, fl);

  ((object_t*)ptr)->flags |= fl;
  return out;
}

bool del_flag(void* ptr, flags fl) {
  assert(ptr && (((uword)ptr) & 7) == 0);

  bool out = has_flag(ptr, fl);

  ((object_t*)ptr)->flags &= ~fl;
  return out;
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

// vector ---------------------------------------------------------------------
usize stencil_height(stencil_t* st);
usize stencil_lidx(stencil_t* st, usize n);

vector_t EmptyVector = {
  .obj={
    .next =NULL,
    .hash =0,
    .flags=0,
    .type =VECTOR,
    .gray =false,
    .black=true
  },
  .len =0,
  .vals=NULL
};

value_t vector_ref(vector_t* xs, usize n) {
  assert(n < xs->len);

  stencil_t* st = xs->vals;

  while (stencil_height(st)) {
    usize h   = st->obj.flags & 7;
    usize i   = n >> (h * 6) & 63;
    value_t v = stencil_nth(st, i);
    st        = as_stencil(v);
  }

  return stencil_nth(st, n&63);
}

vector_t* vector_set(vector_t* xs, usize n, value_t val) {
  assert(n < xs->len);

  
}

vector_t* vector_del(vector_t* xs, usize n);
vector_t* vector_add(vector_t* xs, value_t val);
vector_t* vector_rmv(vector_t* xs);

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

// stencil --------------------------------------------------------------------
stencil_t EmptyStencil = {
  .obj={
    .next=NULL,
    .hash=0,
    .flags=0,
    .gray=false,
    .black=true,
    .type=STENCIL
  },
  .bitmap=0
};

#define MAX_STENCIL_DEPTH 8

stencil_t* allocate_stencil(usize bitmap) {
  usize l = popcnt(bitmap);
  assert(l > 0);

  return allocate(sizeof(stencil_t) + l * sizeof(value_t));
}

void init_stencil(stencil_t* xs, usize h, usize bitmap, value_t* args) {
  assert(h < 8);
  assert(popcnt(bitmap) > 0);
  init_object(&xs->obj, STENCIL, h);
  xs->bitmap = bitmap;
  memcpy(xs->array, args, popcnt(bitmap) * sizeof(value_t));
}

value_t stencil(usize bitmap, value_t* args) {
  stencil_t* new;

  if (popcnt(bitmap) == 0)
    new = &EmptyStencil;

  else {
    new = allocate_stencil(bitmap);
    init_stencil(new, 0, bitmap, args);
  }

  return tag_ptr(new, OBJTAG);
}

usize stencil_height(stencil_t* xs) {
  return xs->obj.flags & 7;
}

usize stencil_idx(stencil_t* xs, usize i) {
  return popcnt(xs->bitmap & ((1 << i) - 1));
}


usize stencil_len(stencil_t* xs) {
  return popcnt(xs->bitmap);
}

bool stencil_has(stencil_t* xs, usize i) {
  return !!(xs->bitmap & ((1 << i)));
}

value_t stencil_nth(stencil_t* xs, usize n) {
  assert(n <= 63);
  assert(n < stencil_len(xs));

  return xs->array[n];
}

value_t stencil_ref(stencil_t* xs, usize i) {
  assert(i <= 63);

  if (stencil_has(xs, i))
    return xs->array[stencil_idx(xs, i)];

  return NOTFOUND;
}

stencil_t* stencil_update(stencil_t* xs, usize rmv, usize add, value_t* args) {
  usize oldmap   = xs->bitmap;
  usize newmap   = (oldmap & ~rmv) | add;
  value_t buffer[popcnt(newmap)];

  for (usize n=0; n<64; n++) {
    usize i = 1 << n;

    if (i & newmap)
      buffer[popcnt(newmap & (i - 1))] = args[popcnt(add & (i - 1))];

    else if (i & oldmap && !(i & rmv))
      buffer[popcnt(newmap & (i - 1))] = xs->array[popcnt(oldmap & (i - 1))];
  }

  stencil_t* out = allocate_stencil(newmap);
  init_stencil(out, 0, newmap, buffer);

  return out;
}
