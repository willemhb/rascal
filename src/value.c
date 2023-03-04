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
    [TUPLE]   = "tuple",
    [LIST]    = "list",
    [VECTOR]  = "vector",
    [DICT]    = "dict",
    [SET]     = "set",
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
static void init_object(object_t* object, type_t type, flags fl) {
  object->next  = LiveObjects;
  LiveObjects   = object;
  object->type  = type;
  object->flags = fl;
  object->black = false;
  object->gray  = true;
  object->hash  = 0;
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

// vector ---------------------------------------------------------------------
static stencil_t* allocate_stencil(usize bm);
static void init_stencil(stencil_t* self, usize h, usize bitmap, value_t* args);
usize stencil_height(stencil_t* st);
usize stencil_bits(stencil_t* st, usize n);
bool stencil_has(stencil_t* xs, usize i);
value_t stencil_nth(stencil_t* xs, usize n);
value_t stencil_ref(stencil_t* xs, usize i);
stencil_t* stencil_update(stencil_t* xs, usize rmv, usize add, value_t* args);

typedef struct {
  usize maxh;
  usize full[8];
  usize extra[8];
} hamt_init_map_t;

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
  .vals=&EmptyStencil
};

static vector_t* allocate_vector(void) {
  return allocate(sizeof(vector_t));
}

static void init_vector(vector_t* self, usize n, stencil_t* st) {
  init_object((object_t*)self, VECTOR, 0);

  self->len  = n;
  self->vals = st;
}

static void calc_hamt_dim(usize n, hamt_init_map_t* m) {
  for (usize i=0; i<8; i++) {
    m->full[m->maxh]  = n >> 6;
    m->extra[m->maxh] = n & 63;

    if (m->full[m->maxh] == 0)
      break;

    n = m->full[m->maxh] + !!m->extra[m->maxh];
    m->maxh++;
  }
}

static usize fill_bitmap(usize n) {
  usize out  = 0;

  for (usize i=0; i<n; i++)
    out |= 1 << i;

  return out;
}

value_t vector(usize n, value_t* args) {
  assert(n <= FIXNUM_MAX);

  vector_t* vec;

  if (n == 0)
    vec = &EmptyVector;

  else {
    hamt_init_map_t dim = {
      .maxh  = 0,
      .full  = { 0, 0, 0, 0, 0, 0, 0, 0 },
      .extra = { 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    calc_hamt_dim(n, &dim);
    stencil_t* st = NULL;

    assert(dim.maxh < 8);

    for (usize h=0; h <= dim.maxh; h++) {
      value_t* theseargs = args;
      usize f;

      for (f=0; f < dim.full[h]; f++) {
	st = allocate_stencil(FULL_SMASK);
	init_stencil(st, h, FULL_SMASK, theseargs);
	args[f] = object(st);
	theseargs += 64;
      }

      if (dim.extra[h]) {
	usize bm = fill_bitmap(dim.extra[h]);
	st = allocate_stencil(bm);
	init_stencil(st, h, bm, theseargs);
	args[f] = object(st);
      }
    }

    vec = allocate_vector();
    init_vector(vec, n, st);
  }

  return object(vec);
}

value_t vector_get(vector_t* xs, usize n) {
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
  assert(n <= xs->len);

  stencil_t* sbuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  usize      ibuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  stencil_t* st         = xs->vals;
  usize      maxheight  = stencil_height(st);

  for (usize i=0; i < maxheight; i++) {
    sbuffer[i] = st;
    ibuffer[i] = stencil_bits(st, n);
    st         = as_stencil(stencil_nth(st, ibuffer[i]));
  }

  st = stencil_update(st, 0, 1 << (n & 63), &val);

  for (usize i=maxheight; i > 0; i--) {
    val          = tag_ptr(st, OBJTAG);
    sbuffer[i-1] = stencil_update(sbuffer[i-1], 0, 1 << (ibuffer[i-1] & 63), &val);
    st           = sbuffer[i-1];
  }

  vector_t* out = allocate_vector();
  init_vector(out, xs->len + n == xs->len, st);

  return out;
}

vector_t* vector_del(vector_t* xs, usize n) {
    assert(n < xs->len);

  stencil_t* sbuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  usize      ibuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  stencil_t* st         = xs->vals;
  usize      maxheight  = stencil_height(st);

  for (usize i=0; i < maxheight; i++) {
    sbuffer[i] = st;
    ibuffer[i] = stencil_bits(st, n);
    st         = as_stencil(stencil_nth(st, ibuffer[i]));
  }

  st = stencil_update(st, 1 << (n & 63), 0, NULL);

  for (usize i=maxheight; i > 0; i--) {
    value_t val  = tag_ptr(st, OBJTAG);
    sbuffer[i-1] = stencil_update(sbuffer[i-1], 0, 1 << (ibuffer[i-1] & 63), &val);
    st           = sbuffer[i-1];
  }

  vector_t* out = allocate_vector();
  init_vector(out, xs->len + n == xs->len, st);

  return out;
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

// dict -----------------------------------------------------------------------
dict_t EmptyDict = {
  .obj={
    .next =NULL,
    .hash =0,
    .flags=0,
    .gray =false,
    .black=true,
    .type =DICT
  },
  .len =0,
  .map =&EmptyStencil,
  .vals=&EmptyStencil
};

static dict_t* allocate_dict(void) {
  return allocate(sizeof(dict_t));
}

static void init_dict(dict_t* self, usize n, stencil_t* map, stencil_t* vals) {
  init_object((object_t*)self, DICT, 0);

  self->len = n;
  self->map = map;
  self->vals= vals;
}

value_t dict(usize n, value_t* args) {
  assert((n & 1) == 0);

  n >>= 1;

  dict_t* out;

  if (n == 0)
    out = &EmptyDict;

  else {
    hamt_init_map_t dim = {
      .maxh  = 0,
      .full  = { 0, 0, 0, 0, 0, 0, 0, 0 },
      .extra = { 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    calc_hamt_dim(n, &dim);
    stencil_t* map = NULL,* vals = NULL;

    assert(dim.maxh < 8);

    

    out = allocate_dict();
    init_dict(out, n, map, vals);
  }
  
  return object(out);
}

tuple_t* dict_nth(dict_t* ks, usize n) {
  assert(n < ks->len);

  stencil_t* st = ks->vals;

  while (stencil_height(st)) {
    usize h   = st->obj.flags & 7;
    usize i   = n >> (h * 6) & 63;
    value_t v = stencil_nth(st, i);
    st        = as_stencil(v);
  }

  return as_tuple(stencil_nth(st, n&63));
}

value_t dict_get(dict_t* ks, value_t k) {
  if (ks->len == 0)
    return NOTFOUND;

  uhash h = hash(k);
  value_t x = NOTFOUND;

  stencil_t* map = ks->map;

  for (;;) {
    usize i = stencil_bits(map, h);
    x       = stencil_ref(map, i);

    if (x == NOTFOUND)
      return x;

    else if (stencil_height(map) == 0)
      break;

    else
      map = as_stencil(x);    
  }

  if (is_fixnum(x)) {
    tuple_t* p = dict_nth(ks, as_fixnum(x));

    if (equal(p->slots[0], k))
      return p->slots[1];

    return NOTFOUND;
  } else {
    assert(is_list(x));
    list_t* ords = as_list(x);

    while (ords->len) {
      tuple_t* p = dict_nth(ks, as_fixnum(ords->head));

      if (equal(p->slots[0], k))
        return p->slots[1];

      ords = ords->tail;
    }

    return NOTFOUND;
  }
}

// set ------------------------------------------------------------------------
value_t stencil_href(stencil_t* xs, usize h);
value_t stencil_iref(stencil_t* xs, usize n);

set_t EmptySet = {
  .obj={
    .next =NULL,
    .hash =0,
    .flags=0,
    .gray =false,
    .black=true,
    .type =SET
  },
  .len =0,
  .map =&EmptyStencil,
  .vals=&EmptyStencil
};

static set_t* allocate_set(void) {
  return allocate(sizeof(set_t));
}

static void init_set(set_t* self, usize n, stencil_t* map, stencil_t* vals) {
  init_object((object_t*)self, SET, 0);

  self->len = n;
  self->map = map;
  self->vals= vals;
}

value_t set(usize n, value_t* args) {
  assert((n & 1) == 0);

  set_t* out;

  if (n == 0)
    out = &EmptySet;

  else {
    
  }

  return object(out);
}

bool set_has(set_t* ks, value_t k) {
  if (ks->len == 0)
    return false;

  uhash h         = hash(k);
  stencil_t* map  = ks->map;
  stencil_t* vals = ks->vals;
  value_t r       = stencil_href(map, h);

  if (r == NOTFOUND)
    return false;

  if (is_fixnum(r))
    return equal(k, stencil_iref(vals, as_fixnum(r)));

  assert(is_list(r));

  list_t* rs = as_list(r);

  while (rs->len) {
    r = stencil_iref(vals, as_fixnum(rs->head));

    if (equal(r, k))
      return true;

    rs = rs->tail;
  }

  return false;
}

set_t* set_add(set_t* ks, value_t k) {
  uhash h = hash(k);

  stencil_t* sbuf[8];
}

set_t* set_del(set_t* xs, usize n) {
    assert(n < xs->len);

  stencil_t* sbuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  usize      ibuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  stencil_t* st         = xs->vals;
  usize      maxheight  = stencil_height(st);

  for (usize i=0; i < maxheight; i++) {
    sbuffer[i] = st;
    ibuffer[i] = stencil_bits(st, n);
    st         = as_stencil(stencil_nth(st, ibuffer[i]));
  }

  st = stencil_update(st, 1 << (n & 63), 0, NULL);

  for (usize i=maxheight; i > 0; i--) {
    value_t val  = tag_ptr(st, OBJTAG);
    sbuffer[i-1] = stencil_update(sbuffer[i-1], 0, 1 << (ibuffer[i-1] & 63), &val);
    st           = sbuffer[i-1];
  }

  set_t* out = allocate_set();
  init_set(out, xs->len + n == xs->len, st);

  return out;
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

usize stencil_bits(stencil_t* xs, usize n) {
  return n >> (stencil_height(xs) * 6) & 63;
}

usize stencil_idx(stencil_t* xs, usize i) {
  return popcnt(xs->bitmap & ((1 << i) - 1));
}

usize stencil_lidx(stencil_t* xs, usize h) {
  return stencil_idx(xs, stencil_bits(xs, h));
}

usize stencil_len(stencil_t* xs) {
  return popcnt(xs->bitmap);
}

bool stencil_has(stencil_t* xs, usize i) {
  return !!(xs->bitmap & ((1 << i)));
}

value_t stencil_nth(stencil_t* xs, usize n) {
  assert(n < stencil_len(xs));

  return xs->array[n];
}

value_t stencil_ref(stencil_t* xs, usize i) {
  if (stencil_has(xs, i))
    return xs->array[stencil_idx(xs, i)];

  return NOTFOUND;
}

value_t stencil_iref(stencil_t* xs, usize n) {
  for (;;) {
    usize i = stencil_bits(xs, n);

    if (i > stencil_len(xs))
      return NOTFOUND;

    else if (stencil_height(xs))
      xs = as_stencil(xs->array[i]);

    else
      return xs->array[i];
  }
}

value_t stencil_href(stencil_t* xs, usize h) {
  for (;;) {
    usize hb = stencil_bits(xs, h);
    
    if (!stencil_has(xs, hb))
      return NOTFOUND;

    value_t x = xs->array[stencil_idx(xs, hb)];

    if (is_stencil(x))
      xs = as_stencil(x);

    else
      return x;
  }
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
