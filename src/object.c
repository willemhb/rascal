#include <string.h>
#include <stdio.h>

#include "object.h"
#include "metaobject.h"
#include "text.h"
#include "memory.h"

// lifetime API ---------------------------------------------------------------
int init_object(void* self, void* ini) {
  extern object_t* LiveObjects;

  assert(self);
  assert(ini);

  object_t* oself = self;
  object_init_t* oini = ini;

  oself->next   = LiveObjects;
  LiveObjects   = oself;
  oself->hash   = oini->hash;
  oself->flags  = oini->flags;
  oself->hashed = oini->hashed;
  oself->frozen = oini->frozen;
  oself->type   = oini->type->type.idno & 0x3f;
  oself->gray   = true;
  oself->black  = false;

  int out = 0;

  if (oini->type->init)
    out = oini->type->init(self, ini);

  return out;
}

void mark_object(object_t* self) {
  if (!self)
    return;

  if (self->black)
    return;

  self->black = true;

  object_type_t* type = (object_type_t*)type_of(self);

  if (type->trace)
    push_gray(self);

  else
    self->gray = true;
}

void free_object(object_t* self) {
  usize dealloc = size_of(self);
  object_type_t* type = (object_type_t*)type_of(self);

  if (type->free)
    type->free(self);

  deallocate(self, dealloc);
}


// symbol ---------------------------------------------------------------------
uint64 SymbolCounter = 1;
symbol_t* SymbolTable = NULL;

typedef struct {
  object_init_t base;
  char* name;
} symbol_init_t;

void  print_symbol(value_t val, port_t* ios);
uhash hash_symbol(object_t* obj);
int   compare_symbols(value_t x, value_t y);
int   init_symbol(void* self, void* ini);
void  trace_symbol(object_t* self);
void  free_symbol(object_t* self);

vtable_t SymbolVtable = {
  .print  =print_symbol,
  .hash   =hash_symbol,
  .compare=compare_symbols
};

object_type_t SymbolType = {
  .type={
    .obj={
      .type=OBJECT_TYPE,
      .frozen=true,
      .gray=true,
      .black=false
    },
    .name="symbol",
    .idno=SYMBOL,
    .vtable=&SymbolVtable
  },
  .size=sizeof(symbol_t),
  .init=init_symbol,
  .trace=trace_symbol,
  .free=free_symbol
};

// sacred methods -------------------------------------------------------------

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

void init_symbol(symbol_t *self, char* name) {
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


// globals --------------------------------------------------------------------
#define MIN_CAP    8ul
#define TABLE_LOAD 0.625

// utilities ------------------------------------------------------------------
usize pad_array_size(usize newct, usize oldcap, usize mincap, double loadf) {
  usize newcap = MAX(oldcap, mincap);
    if (newct > newcap * loadf) {
    do {
      newcap <<= 1;
    } while (newct > newcap * loadf);
  } else if (newct < (newcap >> 1) * loadf && newcap > mincap) {
    do {
      newcap >>= 1;
    } while (newct < (newcap >> 1) * loadf && newcap > mincap);
  }
  return newcap;
}

usize pad_table_size(usize newct, usize oldcap) {
  return pad_array_size(newct, oldcap, MIN_CAP, TABLE_LOAD);
}

// API template macros --------------------------------------------------------
#undef ALIST_API

#define ALIST_API(A, X, mincap)                                         \
  void init_##A(TYPE(A)* A) {                                           \
    A->len   = 0;                                                       \
    A->cap   = pad_array_size(0, 0, mincap, 1.0);                       \
    A->array = allocate(A->cap * sizeof(X));                            \
  }                                                                     \
  void free_##A(TYPE(A)* A) {                                           \
    deallocate(A->array, A->cap * sizeof(X));                           \
    init_##A(A);                                                        \
  }                                                                     \
  void resize_##A(TYPE(A)* A, usize len) {                              \
    usize newc = pad_array_size(len, A->cap, mincap, 1.0);              \
    if (newc != A->cap) {                                               \
      A->array = reallocate(A->array,newc*sizeof(X),A->cap*sizeof(X));	\
      A->cap   = newc;                                                  \
    }                                                                   \
  }                                                                     \
  usize A##_push(TYPE(A)* A, X x) {                                     \
    resize_##A(A, A->len+1);                                            \
    A->array[A->len] = x;                                               \
    return A->len++;                                                    \
  }                                                                     \
  X A##_pop(TYPE(A)* A) {                                               \
    assert(A->len > 0);                                                 \
    X out = A->array[--A->len];                                         \
    resize_##A(A, A->len);                                              \
    return out;                                                         \
  }                                                                     \
  X A##_popn(TYPE(A)* A, usize n) {                                     \
    assert(A->len >= n);                                                \
    X out = A->array[A->len-1];                                         \
    resize_##A(A, (A->len -= n));                                       \
    return out;                                                         \
  }                                                                     \
  usize A##_write(TYPE(A)* A, usize n, X* buf) {                        \
    usize off = A->len;                                                 \
    resize_##A(A, (A->len += n));                                       \
    memcpy(A->array+off, buf, n * sizeof(X));                           \
    return off;                                                         \
  }

ALIST_API(bytes, ubyte, 32);
ALIST_API(values, value_t, 8);
ALIST_API(objects, object_t*, 8);
ALIST_API(buffer, char, 512);

// hash table apis ------------------------------------------------------------
static void init_reader_table(void** table, usize cap) {            
  memset(table, 0, cap*2*sizeof(void*));
  
  for (usize i=0; i < cap; i++) {
    *(int*)&table[i*2]       = EOF;
    *(funcptr*)&table[i*2+1] = NULL;
  }
}

static void** reader_locate(htable_t* htable, int key) {		
  uhash h = hash_uword(key);                                               
  uword m = htable->cap-1;                                            
  usize i = h & m;                                                    
  int ikey;                                                         
  while ((ikey=*(int*)&htable->table[i*2]) != EOF) {
    if (key == ikey)
      break;                                                          
    i = (i+1) & m;                                                    
  }                                                                   
  return &htable->table[i*2];
}

static void rehash_reader_table(void** oldt, usize oldc, usize oldl, void** newt, usize newc) {
  init_reader_table(newt, newc);
  usize m = newc-1;
  
  for (usize i=0, n=0; i<oldc && n<oldl; i++) {
    int k = *(int*)&oldt[i*2];

    if (k == EOF)
      continue;

    funcptr v = *(funcptr*)&oldt[i*2+1];
    uhash h = hash_uword(k);
    usize j = h & m;

    while (*(int*)&newt[j*2] != EOF)
      j = (j+1) & m;

    *(int*)&newt[j*2]       = k;
    *(funcptr*)&newt[j*2+1] = v;

    n++;
  }
}

void init_reader(htable_t* htable) {
  htable->count = 0;
  htable->cap   = pad_table_size(0, 0);
  htable->table = allocate(htable->cap * 2 * sizeof(void*));
  init_reader_table(htable->table, htable->cap);
}

void free_reader(htable_t* htable) {
  deallocate(htable->table, htable->cap * 2 * sizeof(void*));
  init_reader(htable);
}

void resize_reader(htable_t* htable, usize n) {
  usize newc = pad_table_size(n, htable->cap);

  if (newc != htable->cap) {
    void** newt = allocate(newc * 2 * sizeof(void*));
    rehash_reader_table(htable->table, htable->cap, htable->count, newt, newc);
    deallocate(htable->table, htable->cap * 2 * sizeof(void*));
    htable->table = newt;
    htable->cap = newc;
  }
}

funcptr reader_get(htable_t* htable, int key) {
  return reader_locate(htable, key)[1];
}

funcptr reader_set(htable_t* htable, int key, funcptr val) {
  resize_reader(htable, htable->count+1);
  
  void** spc = reader_locate(htable, key);

  if (*(int*)spc == EOF) {
    *(int*)spc = key;
    htable->count++;
  }

  funcptr out = spc[1];
  spc[1] = val;
  return out;
}

funcptr reader_del(htable_t* htable, int key) {
  void** spc = reader_locate(htable, key);
  funcptr out = spc[1];
  
  if (*(int*)spc != EOF) {
    *(int*)spc = EOF;
    spc[1] = NULL;
    resize_reader(htable, --htable->count);
  }

  return out;
}
