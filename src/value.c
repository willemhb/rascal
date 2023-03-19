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
  init_object((object_t*)self, NATIVE, FROZEN);

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

  return object(*node);
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
  init_object(&bin->obj, BINARY, FROZEN);

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

  init_object(&self->obj, TUPLE, FROZEN);
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

  return object(tup);
}

// list -----------------------------------------------------------------------
list_t EmptyList = {
  .obj={
    .next =NULL,
    .type =LIST,
    .hash =0,
    .flags=FROZEN,
    .black=true,
    .gray =false
  },
  .len=0,
  .head=NUL,
  .tail=&EmptyList
};

static void init_list(list_t* self, value_t head, list_t* tail) {
  assert(tail->len < FIXNUM_MAX);
  init_object(&self->obj, LIST, FROZEN);

  self->head = head;
  self->tail = tail;
  self->len  = 1 + tail->len;
}

value_t cons(value_t head, list_t* tail) {
  list_t* out = allocate(sizeof(list_t));
  init_list(out, head, tail);
  return object(out);
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

  return object(out);
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

// vector node ----------------------------------------------------------------
#define VEC_MAXH 8
#define VEC_MAXN 64

vector_node_t* unfreeze_vector_node(vector_node_t* node);

usize pad_vector_node_size(usize n, usize oldc) {
  oldc = MAX(1u, oldc);

  if (n > oldc || n < (oldc >> 1))
    return ceil2(n);

  return oldc;
}

void resize_vector_node(vector_node_t* node, usize n) {
  assert(n <= VEC_MAXN);

  usize padded = pad_vector_node_size(n, node->cap);

  if (padded != node->cap) {
    node->values = reallocate(node->values, node->cap * sizeof(value_t), padded * sizeof(value_t));
    node->cap    = padded;
  }

  node->len = n;
}

void init_vector_node(vector_node_t* self, uint16 len, uint32 height, void* src) {
  init_object(self, VECTOR_NODE, 0);

  self->len    = len;
  self->cap    = pad_vector_node_size(len, 0);
  self->height = height;
  self->values = allocate(self->cap * sizeof(value_t));

  if (src)
    memcpy(self->values, src, self->len * sizeof(value_t));
}

vector_node_t* vector_node(uint16 len, uint32 height, void* src) {
  vector_node_t* out = allocate(sizeof(vector_node_t));

  init_vector_node(out, len, height, src);

  return out;
}

vector_node_t* unfreeze_vector_node(vector_node_t* node) {
  if (has_flag(node, FROZEN))
    node = vector_node(node->len, node->height, node->values);

  return node;
}

void freeze_vector_node(vector_node_t* node) {
  if (has_flag(node, FROZEN))
    return;

  set_flag(node, FROZEN);

  if (node->height)
    for (int i=0; i<node->len; i++)
      freeze_vector_node(node->children[i]);
}

value_t vector_node_get(vector_node_t* node, usize n) {
  while (node->height)
      node = node->children[n >> (node->height * 6) & 0x3f];

  return node->values[n & 0x3f];
}

vector_node_t* vector_node_set(vector_node_t* node, usize n, value_t v) {
  bool frozen = has_flag(node, FROZEN);
  vector_node_t* buffer[VEC_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[VEC_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h;

  for (;;h=node->height) {
    int i = n >> (node->height * 6) & 0x3f;
    indices[h] = i;
    buffer[h] = unfreeze_vector_node(node);
    
    if (h)
      node = node->children[i];

    else
      break;
  }

  for (h=0; h <= maxh; h++) {
    int i = indices[h];
    
    if (h)
      buffer[h]->children[i] = buffer[h-1];

    else
      buffer[h]->values[i] = v;
  }

  if (frozen)
    for (h=0; h <= maxh; h++)
      freeze(buffer[h]);

  return buffer[maxh];
}

vector_node_t* vector_node_append(vector_node_t* node, value_t v) {
  bool frozen = has_flag(node, FROZEN);
  vector_node_t* buffer[VEC_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[VEC_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h;

  for (;;h=node->height) {
    int i = node->len-1;
    indices[h] = i;
    buffer[h]  = node;

    if (h)
      node = node->children[i];

    else
      break;
}

  for (h=0; h<=maxh; h++) {
    if (indices[h] < 63) {
      indices[h]++;
      buffer[h] = unfreeze_vector_node(buffer[h]);
      resize_vector_node(buffer[h], indices[h]+1);

      for (h=h+1;h<=maxh; h++)
        buffer[h] = unfreeze_vector_node(buffer[h]);
    } else {
      vector_node_t* tmp = buffer[h], * new = vector_node(1, h, NULL);

      buffer[h]  = new;
      indices[h] = 0;

      if (h == maxh) {
        buffer[h+1]  = vector_node(1, h+1, &tmp);
        indices[h+1] = 0;
        maxh++;
      }
    }
  }

  for (h=0; h<=maxh; h++) {
    int i = indices[h];

    if (h)
      buffer[h]->children[i] = buffer[h-1];

    else
      buffer[h]->values[i] = v;
  }

  if (frozen)
    for (h=0; h<=maxh; h++)
      freeze(buffer[h]);

  return buffer[maxh];
}

vector_node_t* vector_node_pop(vector_node_t* node) {
  bool frozen = has_flag(node, FROZEN);
  vector_node_t* buffer[VEC_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[VEC_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h, minh = 0;

  for (;;h=node->height) {
    int i = node->len-1;
    indices[h] = i;
    buffer[h]  = node;

    if (h)
      node = node->children[i];

    else
      break;
  }

  for (h=0; h<=maxh; h++) {
    int i = indices[h];

    if (i > 0) {
      if (h == maxh && i == 1) // special case: the root only has one nonempty child, which becomes the new tree
        return buffer[h]->children[0];

      minh = h;
      buffer[h] = unfreeze_vector_node(buffer[h]);
      resize_vector_node(buffer[h], i);

      for (h=h+1;h<=maxh; h++)
        buffer[h] = unfreeze_vector_node(buffer[h]);
    }
  }

  for (h=minh+1; h<=maxh; h++) {
    int i = indices[h];

    buffer[h]->children[i] = buffer[h-1];
  }

  if (frozen)
    for (h=0; h<=maxh; h++)
      freeze(buffer[h]);

  return buffer[maxh];
}

// vector ---------------------------------------------------------------------
void init_vector(vector_t* self, usize n, vector_node_t* root) {
  init_object(self, VECTOR, FROZEN);
  self->len  = n;
  self->root = root;
}
vector_t* mk_vector(usize n, vector_node_t* root) {
  if (n == 0)
    return &EmptyVector;

  vector_t* out = allocate(sizeof(vector_t));

  init_vector(out, n, root);

  return out;
}

value_t vector(usize n, value_t* vals) {
  vector_t* out;
  
  if (n == 0)
    out = &EmptyVector;

  else {
    vector_node_t* root = vector_node(0, 0, NULL);

    for (usize i=0; i<n; i++)
      root = vector_node_append(root, vals[i]);

    out = mk_vector(n, root);
  }

  return object(out);
}

value_t vector_get(vector_t* self, usize n) {
  assert(n < self->len);

  return vector_node_get(self->root, n);
}

vector_t* vector_set(vector_t* self, usize n, value_t v) {
  assert(n < self->len);

  vector_node_t* root = vector_node_set(self->root, n, v);

  return mk_vector(self->len, root);
}

vector_t* vector_add(vector_t* self, value_t v) {
  assert(self->len < FIXNUM_MAX);

  vector_node_t* root = vector_node_append(self->root, v);

  return mk_vector(self->len, root);
}

vector_t* vector_pop(vector_t* self) {
  assert(self->len);
  
  if (self->len == 1)
    return &EmptyVector;

  vector_node_t* root = vector_node_pop(self->root);

  return mk_vector(self->len-1, root);
}

// dict node ------------------------------------------------------------------
// dict -----------------------------------------------------------------------
// set node -------------------------------------------------------------------
#define SET_MAXH 8
#define SET_MAXN 64

set_node_t* unfreeze_set_node(set_node_t* node);

usize pad_set_node_size(usize n, usize oldc) {
  oldc = MAX(1u, oldc);

  if (n > oldc || n < (oldc >> 1))
    return ceil2(n);

  return oldc;
}

void resize_set_node(set_node_t* node, usize n) {
  assert(n <= SET_MAXN);

  usize padded = pad_set_node_size(n, node->cap);

  if (padded != node->cap) {
    node->values = reallocate(node->values, node->cap * sizeof(value_t), padded * sizeof(value_t));
    node->cap    = padded;
  }

  node->len = n;
}

void init_set_node(set_node_t* self, usize bitmap, uint16 len, uint32 height, void* src) {
  init_object(self, SET_NODE, 0);

  if (len == 0)
    len = popcnt(bitmap);

  self->len    = len;
  self->cap    = pad_set_node_size(len, 0);
  self->height = height;
  self->values = allocate(self->cap * sizeof(value_t));

  if (src)
    memcpy(self->values, src, self->len * sizeof(value_t));
}

set_node_t* set_node(usize bitmap, uint16 len, uint32 height, void* src) {
  set_node_t* out = allocate(sizeof(set_node_t));

  init_set_node(out, bitmap, len, height, src);

  return out;
}

set_node_t* unfreeze_set_node(set_node_t* node) {
  if (has_flag(node, FROZEN))
    node = set_node(node->bitmap, node->len, node->height, node->values);

  return node;
}

void freeze_set_node(set_node_t* node) {
  if (has_flag(node, FROZEN))
    return;

  set_flag(node, FROZEN);

  if (node->height)
    for (int i=0; i<node->len; i++)
      freeze_set_node(node->children[i]);
}

static int find_set_node_value(set_node_t* node, value_t v, set_node_t* buf[SET_MAXH], int is[SET_MAXH]) {
  memset(buf, 0, SET_MAXH * sizeof(void*));
  memset(is, -1, SET_MAXH * sizeof(int));

  uhash h = hash(v);

  for (;;) {
    usize l = h >> (node->height * 6) & 0x3f;
    usize m = 1 << l;
    usize i = popcnt(node->bitmap & (m - 1));

    if (buf) {
      buf[node->height] = node;
      is[node->height]  = i;
    }

    if (node->bitmap & m) {
      if (node->height)
        node = node->children[i];

      else if (equal(v, node->values[i]))
        return i;

      else
        return -1;
    } else {
      return -1;
    }
  }
}

bool set_node_has(set_node_t* node, value_t v) {
  int i = find_set_node_value(node, v, NULL, NULL);

  return i > -1;
}

set_node_t* set_node_add(set_node_t* node, value_t v) {
  set_node_t* buffer[SET_MAXH];
  int         indices[SET_MAXH], maxh = node->height;
  bool        frozen = has_flag(node, FROZEN);

  if (find_set_node_value(node, v, buffer, indices) > -1) // nothing to do
    return node;

  int h = 0;
  

  if (frozen)
    for (h=0; h<=maxh; h++)
      freeze(buffer[h]);

  return buffer[maxh];
}

// set ------------------------------------------------------------------------
