#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "value.h"
#include "memory.h"
#include "number.h"
#include "compare.h"

// globals --------------------------------------------------------------------
// symbol table ---------------------------------------------------------------
uword     SymbolCounter = 1;
symbol_t* SymbolTable = NULL;

extern object_t *LiveObjects;

// dispatch tables ------------------------------------------------------------
usize size_of_tuple(void* ptr);
usize size_of_vector(void* ptr);
usize size_of_dict(void* ptr);
usize size_of_set(void* ptr);
usize size_of_binary(void* ptr);

usize (*SizeOf[NUM_TYPES])(void* ptr) = {
  [TUPLE]   = size_of_tuple,
  [VECTOR]  = size_of_vector,
  [DICT]    = size_of_dict,
  [SET]     = size_of_set,
  [BINARY]  = size_of_binary
};

usize BaseSize[NUM_TYPES] = {
  [UNIT]    = sizeof(NUL),
  [BOOL]    = sizeof(bool_t),
  [SYSPTR]  = sizeof(sysptr_t),
  [NATIVE]  = sizeof(native_t),
  [REAL]    = sizeof(real_t),
  [FIXNUM]  = sizeof(fixnum_t),
  [SYMBOL]  = sizeof(symbol_t),
  [TUPLE]   = sizeof(tuple_t),
  [LIST]    = sizeof(list_t),
  [VECTOR]  = sizeof(vector_t),
  [DICT]    = sizeof(dict_t),
  [SET]     = sizeof(set_t),
  [BINARY]  = sizeof(binary_t)
};

usize size_of_tuple(void* ptr) {
  tuple_t* tup = ptr;

  return sizeof(tuple_t) + tup->len * sizeof(value_t);
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
    case SYSPTRTAG: return SYSPTR;
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
    [SYSPTR]  = "sysptr",
    [NATIVE]  = "native",
    [REAL]    = "real",
    [FIXNUM]  = "fixnum",
    [SYMBOL]  = "symbol",
    [BINARY]  = "binary",
    [TUPLE]   = "tuple",
    [LIST]    = "list",
    [STENCIL] = "stencil",
    [VECTOR]  = "vector",
    [DICT]    = "dict",
    [SET]     = "set",
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

bool is_object_type(type_t type) {
  return type > FIXNUM && type < ANY;
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
static void init_object(void* ptr, type_t type, flags fl) {
  object_t* obj = ptr;
  obj->next   = LiveObjects;
  LiveObjects = obj;
  obj->type   = type;
  obj->flags  = fl;
  obj->black  = false;
  obj->gray   = true;
  obj->hash   = 0;
}

// native ---------------------------------------------------------------------
static native_t* allocate_native(void) {
  return allocate(sizeof(native_t));
}

static void init_native(native_t* self, char* name, value_t (*func)(usize n, value_t* args)) {
  init_object((object_t*)self, NATIVE, 0);

  self->name = as_symbol(symbol(name));
  self->func = func;

  self->name->bind = object(self);
}

value_t native(char* name, value_t (*func)(usize n, value_t* args)) {
  native_t* self = allocate_native();
  init_native(self, name, func);
  return object(self);
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
  self->bind  = UNBOUND;
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

value_t pair(value_t k, value_t v) {
  value_t vals[2] = { k, v };

  return tuple(2, vals);
}

value_t tuple(usize n, value_t* args) {
  assert(n <= FIXNUM_MAX);
  tuple_t* tup;

  if (n == 0)
    tup = &EmptyTuple;

  else {
    tup = allocate_tuple(n);
    init_tuple(tup, n, args);
  }

  return tag_ptr(tup, OBJTAG);
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
  assert(tail->len < FIXNUM_MAX);
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
    return object(&EmptyList);

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

list_t* nth_tl(list_t* xs, usize n) {
  assert(n < xs->len);

  while (n--)
    xs = xs->tail;

  return xs;
}

// stencil --------------------------------------------------------------------
stencil_t EmptyStencil = {
  .obj={
    .next =NULL,
    .hash =0,
    .flags=0,
    .black=true,
    .gray =false,
    .type =STENCIL
  },
  .len   =0,
  .height=0,
  .bitmap=0
};

static stencil_t* allocate_stencil(uint len) {
  assert(len <= 64);
  assert(len > 0);

  return allocate(sizeof(stencil_t) + len * sizeof(value_t));
}

static void init_stencil(stencil_t* self, uint len, uint height, usize bitmap, value_t* args) {
  assert(height < 8);
  assert(len <= 64);
  assert(len > 6);
  
  init_object(self, STENCIL, 0);

  self->len    = len;
  self->height = height;
  self->bitmap = bitmap;

  if (args)
    memcpy(self->array, args, len * sizeof(value_t));
}

value_t stencil(uint len, uint height, usize bitmap, value_t* args) {
  stencil_t* out;
  
  if (len == 0) {
    assert(popcnt(bitmap) == 0);
    out = &EmptyStencil;
  } else {
    out = allocate_stencil(len);
    init_stencil(out, len, height, bitmap, args);
  }

  return object(out);
}

// vector ---------------------------------------------------------------------
vector_t EmptyVector = {
  .obj={
    .next =NULL,
    .hash =0,
    .flags=0,
    .black=true,
    .gray =false,
    .type =VECTOR
  },
  .len   =0,
  .map   =&EmptyStencil
};

static vector_t* allocate_vector(void) {
  return allocate(sizeof(vector_t));
}

static void init_vector(vector_t* self, usize n, stencil_t* map) {
  init_object(self, VECTOR, 0);

  self->len = n;
  self->map = map;
}

static vector_t* mk_vector(usize n, stencil_t* map) {
  vector_t* out = allocate_vector(); init_vector(out, n, map);
  
  return out;
}

value_t vector(usize n, value_t* args) {
  (void)n;
  (void)args;
  
  return NUL;
}

value_t vector_get(vector_t* self, usize n) {
  assert(n < self->len);

  stencil_t* map = self->map;

  for (;;) {
    usize i = n >> (map->height * 6) & 63;

    if (map->height)
      map = as_stencil(map->array[i]);

    else
      return map->array[i];
  }
}

vector_t* vector_set(vector_t* self, usize n, value_t val) {
  assert(n < self->len);

  stencil_t* map     = self->map;
  stencil_t* maps[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  usize      idxs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  usize      maxh    = map->height;

  for (;;) {
    usize i = n >> (map->height * 6) & 63;
    maps[map->height] = map;
    idxs[map->height] = i;

    if (map->height)
      map = as_stencil(map->array[i]);

    else
      break;
  }

  for (usize i=0; i<=maxh; i++) {
    map = maps[i] = duplicate(maps[i], size_of(maps[i]));

    if (i)
      map->array[idxs[i]] = object(maps[i-1]);

    else
      map->array[idxs[i]] = val;
  }

  return mk_vector(self->len, map);
}

vector_t* vector_push(vector_t* self, value_t v) {
  
}
