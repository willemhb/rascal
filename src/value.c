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
usize size_of_binary(void* ptr);

usize (*SizeOf[NUM_TYPES])(void* ptr) = {
  [TUPLE]   = size_of_tuple,
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
    .flags=FROZEN,
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

// arr node -------------------------------------------------------------------
#define ARR_MAXH 8
#define ARR_MAXN 64

arr_node_t* unfreeze_arr_node(arr_node_t* node);

usize pad_arr_node_size(usize n, usize oldc) {
  oldc = MAX(1u, oldc);

  if (n > oldc || n < (oldc >> 1))
    return ceil2(n);

  return oldc;
}

void resize_arr_node(arr_node_t* node, usize n) {
  assert(n <= ARR_MAXN);

  usize padded = pad_arr_node_size(n, node->cap);

  if (padded != node->cap) {
    node->values = reallocate(node->values, node->cap * sizeof(value_t), padded * sizeof(value_t));
    node->cap    = padded;
  }

  node->len = n;
}

void init_arr_node(arr_node_t* self, uint16 len, uint32 height, void* src) {
  init_object(self, ARR_NODE, 0);

  self->len    = len;
  self->cap    = pad_arr_node_size(len, 0);
  self->height = height;
  self->values = allocate(self->cap * sizeof(value_t));

  if (src)
    memcpy(self->values, src, self->len * sizeof(value_t));
}

arr_node_t* arr_node(uint16 len, uint32 height, void* src) {
  arr_node_t* out = allocate(sizeof(arr_node_t));

  init_arr_node(out, len, height, src);

  return out;
}

arr_node_t* unfreeze_arr_node(arr_node_t* node) {
  if (has_flag(node, FROZEN))
    node = arr_node(node->len, node->height, node->values);

  return node;
}

void freeze_arr_node(arr_node_t* node) {
  if (has_flag(node, FROZEN))
    return;

  set_flag(node, FROZEN);

  if (node->height)
    for (int i=0; i<node->len; i++)
      freeze_arr_node(node->children[i]);
}

value_t arr_node_get(arr_node_t* node, usize n) {
  while (node->height)
      node = node->children[n >> (node->height * 6) & 0x3f];

  return node->values[n & 0x3f];
}

arr_node_t* arr_node_set(arr_node_t* node, usize n, value_t v) {
  bool frozen = has_flag(node, FROZEN);
  arr_node_t* buffer[ARR_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[ARR_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h;

  for (;;h=node->height) {
    int i = n >> (node->height * 6) & 0x3f;
    indices[h] = i;
    buffer[h] = unfreeze_arr_node(node);
    
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

arr_node_t* arr_node_append(arr_node_t* node, value_t v) {
  bool frozen = has_flag(node, FROZEN);
  arr_node_t* buffer[ARR_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[ARR_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h;

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
      buffer[h] = unfreeze_arr_node(buffer[h]);
      resize_arr_node(buffer[h], indices[h]+1);

      for (h=h+1;h<=maxh; h++)
        buffer[h] = unfreeze_arr_node(buffer[h]);
    } else {
      arr_node_t* tmp = buffer[h], * new = arr_node(1, h, NULL);

      buffer[h]  = new;
      indices[h] = 0;

      if (h == maxh) {
        buffer[h+1]  = arr_node(1, h+1, &tmp);
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

arr_node_t* arr_node_pop(arr_node_t* node) {
  bool frozen = has_flag(node, FROZEN);
  arr_node_t* buffer[ARR_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[ARR_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h, minh = 0;

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
      buffer[h] = unfreeze_arr_node(buffer[h]);
      resize_arr_node(buffer[h], i);

      for (h=h+1;h<=maxh; h++)
        buffer[h] = unfreeze_arr_node(buffer[h]);
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
struct vector_t EmptyVector = {
  .obj ={
    .next =NULL,
    .hash =0,
    .flags=FROZEN,
    .type =VECTOR,
    .gray =false,
    .black=true
  },
  .len =0,
  .root=NULL
};

void init_vector(vector_t* self, usize n, arr_node_t* root) {
  init_object(self, VECTOR, FROZEN);
  self->len  = n;
  self->root = root;
}
vector_t* mk_vector(usize n, arr_node_t* root) {
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
    arr_node_t* root = arr_node(0, 0, NULL);

    for (usize i=0; i<n; i++)
      root = arr_node_append(root, vals[i]);

    out = mk_vector(n, root);
  }

  return object(out);
}

value_t vector_get(vector_t* self, usize n) {
  assert(n < self->len);

  return arr_node_get(self->root, n);
}

vector_t* vector_set(vector_t* self, usize n, value_t v) {
  assert(n < self->len);

  arr_node_t* root = arr_node_set(self->root, n, v);

  return mk_vector(self->len, root);
}

vector_t* vector_add(vector_t* self, value_t v) {
  assert(self->len < FIXNUM_MAX);

  arr_node_t* root = arr_node_append(self->root, v);

  return mk_vector(self->len, root);
}

vector_t* vector_pop(vector_t* self) {
  assert(self->len);
  
  if (self->len == 1)
    return &EmptyVector;

  arr_node_t* root = arr_node_pop(self->root);

  return mk_vector(self->len-1, root);
}

// map node -------------------------------------------------------------------
#define MAP_MAXH 8
#define MAP_MAXN 64

map_node_t* unfreeze_map_node(map_node_t* self);
void        resize_map_node(map_node_t* self, usize n);

usize pad_map_node_size(usize n, usize oldc) {
  oldc = MAX(1u, oldc);

  if (n > oldc || n < (oldc >> 1))
    return ceil2(n);

  return oldc;
}

uhash map_node_key(map_node_t* node, uhash h) {
  return h >> (node->height * 6) & 0x3f;
}

usize map_node_mask(map_node_t* node, uhash h) {
  return 1 << map_node_key(node, h);
}

int map_node_index(map_node_t* node, uhash h) {
  usize mask = map_node_mask(node, h);

  if (node->bitmap & mask)
    return popcnt(node->bitmap & (mask - 1));

  return -1;
}

int add_key_to_map_node(map_node_t* node, uhash h) {
  usize mask = map_node_mask(node, h);

  if (node->bitmap & mask)
    return popcnt(node->bitmap & (mask - 1));

  node->bitmap |= mask;
  int i         = popcnt(node->bitmap & (mask - 1));

  resize_map_node(node, node->len+1);

  if (i < node->len)
    memmove(node->entries+i+1, node->entries+i, (node->len - i) * sizeof(void*));

  node->entries[i] = NULL;

  return i;
}

void resize_map_node(map_node_t* node, usize n) {
  assert(n <= MAP_MAXN);

  usize padded = pad_map_node_size(n, node->cap);

  if (padded != node->cap) {
    node->entries = reallocate(node->entries,
                               node->cap * sizeof(value_t),
                               padded * sizeof(value_t));
    node->cap     = padded;
  }

  node->len = n;
}


void init_map_node(map_node_t* self, usize bitmap, uint16 len, uint32 height, void* src) {
  init_object(self, MAP_NODE, 0);

  if (len == 0)
    len = popcnt(bitmap);

  self->len     = len;
  self->cap     = pad_map_node_size(len, 0);
  self->height  = height;
  self->entries = allocate(self->cap * sizeof(void*));
  self->bitmap  = bitmap;

  if (src)
    memcpy(self->entries, src, self->len * sizeof(void*));
}

map_node_t* map_node(usize bitmap, uint16 len, uint32 height, void* src) {
  map_node_t* out = allocate(sizeof(map_node_t));

  init_map_node(out, bitmap, len, height, src);

  return out;
}

map_node_t* unfreeze_map_node(map_node_t* node) {
  if (has_flag(node, FROZEN))
    node = map_node(node->bitmap, node->len, node->height, node->entries);

  return node;
}

void freeze_map_node(map_node_t* node) {
  if (has_flag(node, FROZEN))
    return;

  set_flag(node, FROZEN);

  if (node->height)
    for (int i=0; i<node->len; i++)
      if (node->entries[i]->type == MAP_NODE)
        freeze_map_node((map_node_t*)node->entries[i]);
}

tuple_t* map_node_get(map_node_t* node, value_t k) {
  uhash h = hash(k); tuple_t* out = NULL;
  
  for (;;) {
    int i = map_node_index(node, h);

    if (i == -1)
      break;

    object_t* o = node->entries[i];
    type_t t    = o->type;

    if (t == MAP_NODE)
      node = (map_node_t*)o;

    else if (t == TUPLE) {
      tuple_t* e = (tuple_t*)o;
      
      if (equal(k, first(e)))
        out = e;

      break;
    }

    else {
      assert(t == LIST);
      list_t* l = (list_t*)o;

      while (!out && l->len) {
        tuple_t* e = as_ptr(l->head);

        if (equal(k, first(e)))
          out = e;

        else
          l = l->tail;
      }

      break;
    }
  }

  return out;
}

bool map_node_set(map_node_t** root, value_t k, value_t v) {
  map_node_t* node              = *root;
  map_node_t* buffer[MAP_MAXH]  = {  0,  0,  0,  0,  0,  0,  0,  0 };
  int         indices[MAP_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
  bool        frozen            = has_flag(node, FROZEN), out = false;
  uhash       h                 = hash(k);
  int         height            = node->height, maxh = height, minh = height, i;
  object_t*   entry             = NULL;

  // find path to entry node
  for (;;height=node->height, minh=height) {
    buffer[height]  = node;
    indices[height] = i = map_node_index(node, h);

    if (i == -1) {
      buffer[height]  = unfreeze_map_node(buffer[height]);
      indices[height] = i = add_key_to_map_node(node, h);
      break;
    }

    object_t* o = node->entries[i];

    if (o->type == MAP_NODE)
      node = (map_node_t*)o;

    // 
    else if (o->type == TUPLE) {
      
    }

    else {
      
    }
  }
}

bool map_node_del(map_node_t** node, value_t k);

// dict -----------------------------------------------------------------------
struct dict_t EmptyDict = {
  .obj ={
    .next =NULL,
    .hash =0,
    .flags=FROZEN,
    .type =DICT,
    .gray =false,
    .black=true
  },
  .len =0,
  .root=NULL
};

void init_dict(dict_t* self, usize n, map_node_t* root) {
  init_object(self, DICT, FROZEN);

  self->len = n;
  self->root= root;
}

dict_t* mk_dict(usize n, map_node_t* root) {
  dict_t* out = allocate(sizeof(dict_t));

  init_dict(out, n, root);

  return out;
}

value_t dict(usize n, value_t* values) {
  assert((n&1) == 0);
  
  dict_t* out;
  
  if (n == 0)
    out = &EmptyDict;

  else {
    map_node_t* root = map_node(0, 0, 0, NULL);
    usize len = 0;

    for (usize i=0; i<n; i+= 2)
      len += map_node_set(&root, values[i], values[i+1]);

    freeze_map_node(root);

    out = mk_dict(len, root);
  }

  return object(out);
}

value_t dict_get(dict_t* ks, value_t k) {
  tuple_t* kv = map_node_get(ks->root, k);

  if (kv == NULL)
    return NOTFOUND;

  return second(kv);
}

dict_t* dict_set(dict_t* ks, value_t k, value_t v) {
  map_node_t* root = ks->root;
  usize len = ks->len;

  len += map_node_set(&root, k, v);

  return mk_dict(len, root);
}

dict_t* dict_del(dict_t* ks, value_t k) {
  map_node_t* root = ks->root;
  usize len = ks->len;
  len -= map_node_del(&root, k);

  return mk_dict(len, root);
}

// set ------------------------------------------------------------------------
struct set_t EmptySet = {
  .obj ={
    .next =NULL,
    .hash =0,
    .flags=FROZEN,
    .type =SET,
    .gray =false,
    .black=true
  },
  .len =0,
  .root=NULL
};


void init_set(set_t* self, usize n, map_node_t* root) {
  init_object(self, SET, FROZEN);

  self->len = n;
  self->root= root;
}

set_t* mk_set(usize n, map_node_t* root) {
  set_t* out = allocate(sizeof(set_t));

  init_set(out, n, root);

  return out;
}

value_t set(usize n, value_t* values) {
  assert((n&1) == 0);
  
  set_t* out;
  
  if (n == 0)
    out = &EmptySet;

  else {
    map_node_t* root = map_node(0, 0, 0, NULL);
    usize len = 0;

    for (usize i=0; i<n; i+= 2)
      len += map_node_set(&root, values[i], values[i+1]);

    freeze_map_node(root);

    out = mk_set(len, root);
  }

  return object(out);
}

bool set_has(set_t* ks, value_t k) {
  return !!map_node_get(ks->root, k);
}

set_t* set_add(set_t* ks, value_t k) {
  map_node_t* root = ks->root;
  usize len = ks->len;

  len += map_node_set(&root, k, k);

  return mk_set(len, root);
}

set_t* set_del(set_t* ks, value_t k) {
  map_node_t* root = ks->root;
  usize len = ks->len;
  len -= map_node_del(&root, k);

  return mk_set(len, root);
}
