#include <string.h>
#include <stdio.h>

#include "object.h"
#include "metaobject.h"
#include "text.h"
#include "atom.h"
#include "interpreter.h"

#include "memory.h"
#include "number.h"
#include "hashing.h"

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

void mark_object(void* self) {
  if (!self)
    return;

  object_t* obj = self;

  if (obj->black)
    return;

  obj->black = true;

  object_type_t* type = (object_type_t*)type_of(obj);

  if (type->trace)
    push_gray(obj);

  else
    obj->gray = true;
}

void free_object(void* self) {
  if (!self)
    return;
  
  object_t* obj = self;
  usize dealloc = size_of(obj);
  object_type_t* type = (object_type_t*)type_of(obj);

  if (type->free)
    type->free(obj);

  deallocate(obj, dealloc);
}

// traversal utilities --------------------------------------------------------
void mark_objects(usize n, object_t** objs) {
  for (usize i=0; i<n; i++)
    mark_object(objs[i]);
}

void mark_values(usize n, value_t* vals) {
  for (usize i=0; i<n; i++)
    mark_value(vals[i]);
}

int compare_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs) {
  usize maxc = MAX(xn, yn);

  int o;

  for (usize i=0; i<maxc; i++) {
    if ((o=rl_compare(xobjs[i], yobjs[i])))
      return o;
  }

  return 0 - (xn < yn) + (xn > yn);
}

int compare_values(usize xn, value_t* xvals, usize yn, value_t* yvals) {
  usize maxc = MAX(xn, yn);

  int o;

  for (usize i=0; i<maxc; i++) {
    if ((o=rl_compare(xvals[i], yvals[i])))
      return o;
  }

  return 0 - (xn < yn) + (xn > yn);
}

bool equal_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs) {
  if (xn != yn)
    return false;

  for (usize i=0; i<xn; i++) {
    if (!rl_equal(xobjs[i], yobjs[i]))
      return false;
  }

  return true;
}

bool equal_values(usize xn, value_t* xvals, usize yn, value_t* yvals) {
  if (xn != yn)
    return false;

  for (usize i=0; i<xn; i++) {
    if (!rl_equal(xvals[i], yvals[i]))
      return false;
  }

  return true;
}

uhash hash_objects(usize nx, object_t** objs) {
  uhash accum = 0;

  for (usize i=0; i<nx; i++)
    accum = mix_2_hashes(accum, rl_hash(objs[i]));

  return accum;
}

uhash hash_values(usize nx, value_t* vals) {
  uhash accum = 0;

  for (usize i=0; i<nx; i++)
    accum = mix_2_hashes(accum, rl_hash(vals[i]));

  return accum;
}


// symbol ---------------------------------------------------------------------
uint64 SymbolCounter = 1;
symbol_t* SymbolTable = NULL;

typedef struct {
  object_init_t base;
  char* name;
  symbol_t** parent;
} symbol_init_t;

void  print_symbol(value_t val, port_t* ios);
uhash hash_symbol(object_t* obj);
int   compare_symbols(value_t x, value_t y);
int   init_symbol(void* self, void* ini);
void  trace_symbol(void* self);
void  free_symbol(void* self);

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
void print_symbol(value_t val, port_t* ios) {
  symbol_t* s = as_pointer(val);

  if (has_flag(s, INTERNED))
    rl_printf(ios, "%s", s->name);

  else
    rl_printf(ios, "%s#%lu", s->name, s->idno);
}

uhash hash_symbol(object_t* obj) {
  symbol_t* s = (symbol_t*)obj;

  uhash baseh = SymbolType.type.obj.hash;
  uhash idnoh = hash_uword(s->idno);
  uhash nameh = hash_str(s->name);

  return mix_3_hashes(baseh, idnoh, nameh);
}

int compare_symbols(value_t x, value_t y) {
  int o;

  symbol_t* sx = as_pointer(x), * sy = as_pointer(y);

  if ((o=strcmp(sx->name, sy->name)))
    return o;

  if ((o=CMP(sx->idno, sy->idno)))
    return o;

  return 0;
}

// lifetime methods -----------------------------------------------------------
int init_symbol(void* self, void* ini) {
  symbol_t* sym = self;
  symbol_init_t* sini = ini;

  sym->idno     = SymbolCounter++;
  sym->name     = duplicate(sini->name, strlen(sini->name)+1);
  sym->left     = NULL;
  sym->right    = NULL;
  sym->toplevel = NULL;

  if (sini->parent)
    *(sini->parent) = sym;

  return 0;
}

void mark_symbol(void* self) {
  symbol_t* sym = self;

  mark_object(sym->left);
  mark_object(sym->right);
  mark_object(sym->toplevel);
}

void free_symbol(void* self) {
  symbol_t* sym = self;

  deallocate(sym->name, strlen(sym->name)+1);
}

// ctor methods ---------------------------------------------------------------
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

static symbol_t *new_symbol(char* name, symbol_t** parent) {
  symbol_t *sym = allocate(sizeof(symbol_t));

  symbol_init_t ini = {
    .base={
      .type=&SymbolType,
      .flags=!!parent * INTERNED,
      .frozen=true
    },
    .name=name,
    .parent=parent
  };

  init_object(sym, &ini);

  return sym;
}

value_t symbol(char* name, bool interned) {
  if (name == NULL) {
    assert(!interned);
    name = "symbol";
  }

  if (interned) {
    symbol_t **node = find_symbol(name);

    if (*node == NULL)
      return object(new_symbol(name, node));

    return object(*node);
  }

  else
    return object(new_symbol(name, NULL));
}

bool is_defined(symbol_t* sym, namespace_t* ns) {
  return lookup(sym, ns) != NULL;
}

bool is_bound(symbol_t* sym, namespace_t* ns) {
  variable_t* v = lookup(sym, ns);

  return v && v->bind != UNBOUND;
}

value_t toplevel(symbol_t* sym) {
  variable_t* v = lookup(sym, NULL);

  if (v)
    return v->bind;

  return UNDEFINED;
}

variable_t* defvar(value_t name, namespace_t* ns, string_t* doc, rl_type_t* type, value_t bind) {
  if (is_pointer(name))
    name = symbol(as_pointer(name), true);

  variable_t* out = variable(ns, as_symbol(name), doc, type);
  out->bind       = bind;

  return out;
}

variable_t* defconst(value_t name, namespace_t* ns, string_t* doc, rl_type_t* type, value_t bind) {
  variable_t* out = defvar(name, ns, doc, type, bind);
  freeze(out);
  return out;
}

// list -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
list_t EmptyList = {
  .obj={
    .type  =LIST,
    .frozen=true
  },
  .len=0,
  .head=NUL,
  .tail=&EmptyList
};

typedef struct {
  object_init_t base;
  value_t head;
  list_t* tail;
} list_init_t;

void  print_list(value_t x, port_t* ios);
uhash hash_list(object_t* o);
bool  equal_lists(object_t* x, object_t* y);
int   compare_lists(value_t x, value_t y);
int   init_list(void* self, void* ini);
void  trace_list(void* self);

vtable_t ListVtable = {
  .print  =print_list,
  .hash   =hash_list,
  .equal  =equal_lists,
  .compare=compare_lists
};

object_type_t ListType = {
  .type={
    .obj={
      .type  =OBJECT_TYPE,
      .frozen=true
    },
    .name  ="list",
    .idno  =LIST,
    .vtable=&ListVtable
  },
  .init     =init_list,
  .trace    =trace_list,
  .singleton=&EmptyList.obj
};

// sacred methods -------------------------------------------------------------
void print_list(value_t val, port_t* ios) {
  rl_putc(ios, '(');

  list_t* xs = as_list(val);

  while (xs->len) {
    rl_print(xs->head, ios);

    xs = xs->tail;

    if (xs->len)
      rl_putc(ios, ' ');
  }

  rl_putc(ios, ')');
}

uhash hash_list(object_t* obj) {
  list_t* lx = (list_t*)obj, * node = lx, * tmp, * prev = NULL;

  uhash lthash = ListType.type.obj.hash;

  while (!node->obj.hashed) {
    // temporarily reset tail to point backward (to traverse back up) -------
    tmp        = node->tail;
    node->tail = prev;
    prev       = node;
    node       = tmp;
  }

  while (node != NULL) {
    uhash headh = rl_hash(node->head);
  }

  return lx->obj.hash;
}

bool equal_lists(object_t* x, object_t* y) {
  list_t* lx = (list_t*)x,* ly = (list_t*)y;

  if (lx->len != ly->len)
    return false;

  while (lx->len) {
    if (!rl_equal(lx->head, ly->head))
      return false;

    lx = lx->tail, ly = ly->tail;
  }

  return true;
}

int compare_lists(value_t x, value_t y) {
  list_t* lx   = as_list(x),* ly = as_list(y);
  usize maxcmp = MIN(lx->len, ly->len);
  int o;

  for (usize i=0; i<maxcmp; i++)
    if (o=rl_compare(lx->head, ly->head))
      return o;

  return 0 - !!lx->len - !!ly->len;
}

// lifetime methods -----------------------------------------------------------
int init_list(void* self, void* ini) {
  list_t* xs = self;
  list_init_t* lini = ini;

  xs->head = lini->head;
  xs->len  = lini->tail->len+1;
  xs->tail = lini->tail;

  return 0;
}

void trace_list(void* ptr) {
  list_t* list = ptr;

  mark_value(list->head);
  mark_object(list->tail);
}

// ctors ----------------------------------------------------------------------
value_t cons(value_t head, list_t* tail) {
  list_t* out = allocate(sizeof(list_t));
  list_init_t ini = {
    .base={
      .type=&ListType,
      .frozen=true,
      .data=true,
      .safe=true
    },
    .head=head,
    .tail=tail
  };

  init_object(out, &ini);
  
  return object(out);
}

value_t list(usize n, value_t* args) {
  if (n == 0)
    return object(&EmptyList);

  if (n == 1)
    return cons(args[0], &EmptyList);

  list_t* out  = allocate(n * sizeof(list_t));
  list_t* curr = &out[n-1], *last = &EmptyList;

  list_init_t ini = {
    .base={
      .type=&ListType,
      .frozen=true,
      .data=true,
      .safe=true
    }
  };

  for (usize i=n; i>0; i--) {
    ini.head = args[i];
    ini.tail = last;
    init_object(curr, &ini);
    last = curr--;
  }

  return object(out);
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

// initialization -------------------------------------------------------------
void object_init(void) {
  rl_hash(&EmptyList.obj);
}
