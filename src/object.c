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

  data_type_t* type = (data_type_t*)type_of(obj);

  if (type->trace)
    push_gray(obj);

  else
    obj->gray = true;
}

void free_object(void* self) {
  if (!self)
    return;
  
  object_t* obj     = self;
  usize dealloc     = rl_size_of(obj);
  data_type_t* type = (data_type_t*)type_of(obj);

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

void print_values(usize n, value_t* vals, port_t* ios, bool paired) {
  for (usize i=0; i<n; i++) {
    if (paired && vals[i] == NOTFOUND)
      continue;
      
    rl_print(vals[i], ios);
    
    if (i + 1 < n) {
      if (paired && i > 0 && i % 2 == 0)
        rl_putc(',', ios);
      
      rl_putc(' ', ios);
    }
  }
}

void print_objects(usize n, object_t** objs, port_t* ios, bool paired) {
  for (usize i=0; i<n; i++) {
    if (paired && objs[i] == NULL)
      continue;

    rl_print(objs[i], ios);

    if (i + 1 < n) {
      if (paired && i > 0 && i % 2 == 0)
        rl_putc(',', ios);
    
      rl_putc(' ', ios);
    }
  }
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
uhash hash_symbol(void* ptr);
int   compare_symbols(value_t x, value_t y);
int   init_symbol(void* self, void* ini);
void  trace_symbol(void* self);
void  free_symbol(void* self);

data_type_t SymbolType = {
  .type={
    .obj={
      .type  =DATA_TYPE,
      .frozen=true,
      .gray  =true
    },
    .name="symbol",
    .idno=SYMBOL,
  },
  .size   =sizeof(symbol_t),
  .print  =print_symbol,
  .hash   =hash_symbol,
  .compare=compare_symbols,
  .init   =init_symbol,
  .trace  =trace_symbol,
  .free   =free_symbol
};

// sacred methods -------------------------------------------------------------
void print_symbol(value_t val, port_t* ios) {
  symbol_t* s = as_pointer(val);

  if (has_flag(s, INTERNED))
    rl_puts(s->name, ios);

  else
    rl_printf(ios, "%s#%lu", s->name, s->idno);
}

uhash hash_symbol(void* ptr) {
  symbol_t* s = ptr;

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
      .type  =&SymbolType,
      .flags =!!parent * INTERNED,
      .frozen=true
    },
    .name  =name,
    .parent=parent
  };

  init_object(sym, &ini);

  return sym;
}

symbol_t* symbol(char* name, bool interned) {
  if (name == NULL) {
    assert(!interned);
    name = "symbol";
  }

  if (interned) {
    symbol_t **node = find_symbol(name);

    if (*node == NULL)
      return new_symbol(name, node);

    return *node;
  }

  else
    return new_symbol(name, NULL);
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

variable_t* defvar(value_t name, namespace_t* ns, string_t* doc, type_t* type, value_t bind) {
  if (is_pointer(name))
    name = object(symbol(as_pointer(name), true));

  variable_t* out = variable(ns, as_symbol(name), doc, type);
  out->bind       = bind;

  return out;
}

variable_t* defconst(value_t name, namespace_t* ns, string_t* doc, type_t* type, value_t bind) {
  variable_t* out = defvar(name, ns, doc, type, bind);
  freeze(out);
  return out;
}

// list -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
list_t EmptyList = {
  .obj={
    .type  =LIST,
    .frozen=true,
    .gray  =true
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
uhash hash_list(void* ptr);
bool  equal_lists(void* xp, void* yp);
int   compare_lists(value_t x, value_t y);
int   init_list(void* self, void* ini);
void  trace_list(void* self);

data_type_t ListType = {
  .type={
    .obj={
      .type  =DATA_TYPE,
      .frozen=true,
      .gray  =true
    },
    .name  ="list",
    .idno  =LIST,
  },
  .size =sizeof(list_t),
  .member=&EmptyList.obj,

  .print  =print_list,
  .hash   =hash_list,
  .equal  =equal_lists,
  .compare=compare_lists,
  .init   =init_list,
  .trace  =trace_list
};

// sacred methods -------------------------------------------------------------
void print_list(value_t val, port_t* ios) {
  rl_putc('(', ios);

  list_t* xs = as_list(val);

  while (xs->len) {
    rl_print(xs->head, ios);

    xs = xs->tail;

    if (xs->len)
      rl_putc(' ', ios);
  }

  rl_putc(')', ios);
}

uhash hash_list(void* ptr) {
  list_t* lx = ptr, * node = lx, * tmp, * prev = NULL;

  uhash lthash = ListType.type.obj.hash;

  if (lx->len == 0)
    return mix_2_hashes(lthash, lthash);

  while (!node->obj.hashed) {
    // temporarily reset tail to point backward (to traverse back up) -------
    tmp        = node->tail;
    node->tail = prev;
    prev       = node;
    node       = tmp;
  }

  while (node != NULL) {
    prev->obj.hash   = mix_3_hashes(lthash, rl_hash(prev->head), node->obj.hash);
    prev->obj.hashed = true;
    tmp              = prev->tail;
    prev->tail       = node;
    node             = prev;
    prev             = tmp;
  }

  return lx->obj.hash;
}

bool equal_lists(void* px, void* py) {
  list_t* lx = px,* ly = py;

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
    if ((o=rl_compare(lx->head, ly->head)))
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
list_t* cons(value_t head, list_t* tail) {
  list_t* out = allocate(sizeof(list_t));
  list_init_t ini = {
    .base={
      .type  =&ListType,
      .frozen=true,
      .data  =true,
      .safe  =true
    },
    .head=head,
    .tail=tail
  };

  init_object(out, &ini);
  
  return out;
}

list_t* list(usize n, value_t* args) {
  if (n == 0)
    return &EmptyList;

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

  return out;
}

// tuple ----------------------------------------------------------------------
// globals --------------------------------------------------------------------
tuple_t EmptyTuple = {
  .obj={
    .type  =TUPLE,
    .frozen=true,
    .gray  =true
  },
  .len=0
};

typedef struct {
  object_init_t base;
  usize len;
  value_t* slots;
} tuple_init_t;

void  print_tuple(value_t x, port_t* ios);
usize tuple_size(void* ptr);
uhash hash_tuple(void* ptr);
bool  equal_tuples(void* px, void* py);
int   compare_tuples(value_t x, value_t y);
int   init_tuple(void* ptr, void* dat);
void  trace_tuple(void* ptr);

data_type_t TupleType = {
  .type={
    .obj={
      
    },
    .name="tuple",
    .idno=TUPLE
  },
  .size   =sizeof(tuple_t),
  .member =&EmptyTuple.obj,

  .print  =print_tuple,
  .size_of=tuple_size,
  .hash   =hash_tuple,
  .equal  =equal_tuples,
  .compare=compare_tuples,
  .init   =init_tuple,
  .trace  =trace_tuple
};

// sacred methods -------------------------------------------------------------
void print_tuple(value_t val, port_t* ios) {
  rl_putc('[', ios);

  tuple_t* xs = as_tuple(val); print_values(xs->len, xs->slots, ios, false);

  rl_putc(']', ios);
}

usize tuple_size(void* ptr) {
  tuple_t* self = ptr;

  return self->len * sizeof(value_t);
}

uhash hash_tuple(void* ptr) {
  tuple_t* self = ptr;
  uhash tth = TupleType.type.obj.hash;

  if (self->len)
    return mix_2_hashes(tth, tth);

  else
    return mix_2_hashes(tth, hash_values(self->len, self->slots));
}

bool equal_tuples(void* px, void* py) {
  tuple_t* xt = px,* yt = py;

  return equal_values(xt->len, xt->slots, yt->len, yt->slots);
}

int compare_tuples(value_t x, value_t y) {
  tuple_t* tx  = as_tuple(x),* ty = as_tuple(y);

  return compare_values(tx->len, tx->slots, ty->len, ty->slots);
}

// lifetime methods -----------------------------------------------------------
int init_tuple(void* spc, void* ini) {
  tuple_t* self = spc;
  tuple_init_t* tini = ini;
  
  assert(tini->len > 0);
  assert(self != &EmptyTuple);

  self->len = tini->len;

  if (tini->slots)
    memcpy(self->slots, tini->slots, self->len * sizeof(value_t));

  return 0;
}

void trace_tuple(void* ptr) {
  tuple_t* tuple = ptr;

  mark_values(tuple->len, tuple->slots);
}

// ctors ----------------------------------------------------------------------
static tuple_t* allocate_tuple(usize n) {
  assert(n > 0);

  return allocate(sizeof(tuple_t) + n * sizeof(value_t));
}


tuple_t* tuple(usize n, value_t* args) {
  assert(n <= FIXNUM_MAX);
  tuple_t* tup;

  if (n == 0)
    return &EmptyTuple;

  
  tuple_init_t ini = {
    .base={
      .type  =&TupleType,
      .frozen=true,
      .data  =true,
      .safe  =true,
    },
    .len   =n,
    .slots=args
  };
  
  tup = allocate_tuple(n);
  
  init_object(tup, &ini);

  return tup;
}

// table ----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define MIN_CAP    8ul
#define MAX_CAP    0x0000800000000000ul
#define TABLE_LOAD 0.625

void print_table(value_t x, port_t* ios);
bool equal_tables(void* px, void* py);
int  compare_tables(value_t x, value_t y);
int  init_table(void* ptr, void* ini);
void trace_table(void* ptr);
void free_table(void* ptr);

data_type_t TableType = {
  .type={
    .obj={
      .type  =DATA_TYPE,
      .frozen=true,
      .gray  =true
    },
    .name="table",
    .idno=TABLE
  },
  .size   =sizeof(table_t),
  .print  =print_table,
  .equal  =equal_tables,
  .compare=compare_tables,
  .init   =init_table,
  .trace  =trace_table,
  .free   =free_table
};

// utilities ------------------------------------------------------------------
usize pad_alist_size(usize newct, usize oldct, usize oldcap, usize mincap) {
  /* CPython resize algorithm */
  usize newcap = MAX(oldcap, mincap);

  if (newcap >= newct && (newct >= (newcap >> 1) || newcap == mincap))
    return newcap;

  newcap = (newct + (newct >> 3) + 6) & ~3ul;

  if (newct - oldct > newcap - newct)
    newcap = (newct + 3) & ~3ul;

  return newcap;
}

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

static usize ord_size(usize cap) {
  assert(cap <= MAX_CAP);
  assert(cap >= MIN_CAP);

  if (cap <= INT8_MAX)
    return cap;

  if (cap <= INT16_MAX)
    return cap * sizeof(sint16);

  if (cap <= INT32_MAX)
    return cap * sizeof(sint32);

  return cap * sizeof(sint64);
}

void* allocate_ord(usize cap) {
  usize total = ord_size(cap);
  void* out   = allocate(total);

  memset(out, -1, total);

  return out;
}

void* reallocate_ord(void* ptr, usize newc, usize oldc) {
  usize oldt = ord_size(oldc), newt = ord_size(newc);

  ptr = reallocate(ptr, newt, oldt);

  memset(ptr, -1, newt);

  return ptr;
}

void deallocate_ord(void* ptr, usize cap) {
  deallocate(ptr, ord_size(cap));
}

value_t* allocate_table(usize cap) {
  value_t* out = allocate(cap * 2 * sizeof(value_t));

  for (usize i=0; i<cap*2; i++)
    out[i] = NOTFOUND;

  return out;
}

value_t* reallocate_table(value_t* ptr, usize newc, usize oldc) {
  value_t* out = reallocate(ptr, newc*2*sizeof(value_t), oldc*2*sizeof(value_t));

  for (usize i=oldc*2; i<newc*2; i++)
    out[i] = NOTFOUND;

  return out;
}

void deallocate_table(value_t* ptr, usize cap) {
  deallocate(ptr, cap * 2 * sizeof(value_t));
}

void rehash_table(table_t* table) {
  value_t* entries = table->table;
  usize count      = table->count;
  usize ocap       = table->ocap;

  if (ocap <= INT8_MAX) {
    sint8* ord = table->ord8;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  } else if (ocap <= INT16_MAX) {
    sint16* ord = table->ord16;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  } else if (ocap <= INT32_MAX) {
    sint32* ord = table->ord32;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  } else {
    sint64* ord = table->ord64;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  }
}

void resize_table(table_t* self, usize cnt) {
  usize tcap = pad_alist_size(cnt, self->count, self->tcap, MIN_CAP);

  if (tcap != self->tcap) {
    self->table = reallocate_table(self->table, tcap, self->tcap);
    self->tcap  = tcap;
  }

  usize ocap = pad_table_size(cnt, self->ocap);

  if (ocap != self->ocap) {
    self->ord  = reallocate_ord(self->ord, ocap, self->ocap);
    self->ocap = ocap;
    rehash_table(self);
  }
}

long table_locate(table_t* self, value_t key, usize* buffer) {
  value_t* table = self->table;
  usize    ocap  = self->ocap;
  usize    omask = ocap - 1;
  uhash    khash = rl_hash(key);
  usize    order = khash & omask;
  long     index = -1;

  if (ocap <= INT8_MAX) {
    sint8* ord = self->ord8;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order =  (order + 1) & omask;
    }
  } else if (ocap <= INT16_MAX) {
    sint16* ord = self->ord16;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order = (order + 1) & omask;
    }
  } else if (ocap <= INT32_MAX) {
    sint32* ord = self->ord32;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order = (order + 1) & omask;
    }
  } else {
    sint64* ord = self->ord64;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order = (order + 1) & omask;
    }
  }

  if (buffer)
    *buffer = order;

  return index;
}

// sacred methods -------------------------------------------------------------
void print_table(value_t x, port_t* ios) {
  rl_puts("#table(", ios);

  table_t* tab = as_table(x); print_values(tab->count*2, tab->table, ios, true);

  rl_putc(')', ios);
}

bool equal_tables(void* xp, void* yp) {
  table_t* xt = xp, * yt = yp;

  return equal_values(xt->count*2, xt->table, yt->count*2, yt->table);
}

int compare_tables(value_t x, value_t y) {
  table_t* xt = as_table(x), * yt = as_table(y);

  return compare_values(xt->count*2, xt->table, yt->count*2, yt->table);
}

// lifetime methods -----------------------------------------------------------
int init_table(void* ptr, void* dat) {
  (void)dat;

  table_t* self = ptr;
  self->count   =   0;
  self->tcap    = pad_alist_size(0, 0, 0, 8);
  self->ocap    = pad_table_size(0, 0);
  self->table   = allocate_table(self->tcap);
  self->ord     = allocate_ord(self->ocap);

  return 0;
}

void trace_table(void* ptr) {
  table_t* self = ptr;

  mark_values(self->count*2, self->table);
}

void free_table(void* ptr) {
  table_t* self = ptr;

  deallocate_ord(self->ord, self->ocap);
  deallocate_table(self->table, self->tcap);
}

// misc -----------------------------------------------------------------------
void reset_table(table_t* self) {
  free_table(self);
  init_table(self, NULL);
}

// ctors ----------------------------------------------------------------------
table_t* table(void) {
  object_init_t ini = {
    .type  =&TableType
  };

  table_t* out = allocate(sizeof(table_t));

  init_object(out, &ini);

  return out;
}

// getters & setters ----------------------------------------------------------
value_t table_get(table_t* self, value_t key) {
  long index = table_locate(self, key, NULL);

  if (index == -1)
    return NOTFOUND;

  return self->table[(index << 1) + 1];
}

value_t table_set(table_t* self, value_t key, value_t val) {
  value_t out; usize order;

  resize_table(self, self->count+1); // preemptively, so that hashed index is valid
  
  long index = table_locate(self, key, &order);

  if (index == -1) {
    out = NOTFOUND;

    if (self->ocap <= INT8_MAX)       self->ord8[order]  = self->count;
    else if (self->ocap <= INT16_MAX) self->ord16[order] = self->count;
    else if (self->ocap <= INT32_MAX) self->ord32[order] = self->count;
    else                              self->ord64[order] = self->count;

    self->table[self->count*2]   = key;
    self->table[self->count*2+1] = val;
    self->count++;
  } else {
    out                    = self->table[index*2+1];
    self->table[index*2+1] = val;
  }

  return out;
}

value_t table_del(table_t* self, value_t key) {
  value_t out; usize order;

  long index = table_locate(self, key, &order);

  if (index == -1)
    out = NOTFOUND;

  else {
    if (self->ocap <= INT8_MAX)       self->ord8[order] =  -1;
    else if (self->ocap <= INT16_MAX) self->ord16[order] = -1;
    else if (self->ocap <= INT32_MAX) self->ord32[order] = -1;
    else                              self->ord64[order] = -1;

    self->table[order*2]   = NOTFOUND;
    self->table[order*2+1] = NOTFOUND;

    resize_table(self, self->count-1);
  }

  return out;
}

// alist ----------------------------------------------------------------------
// globals --------------------------------------------------------------------

void print_alist(value_t self, port_t* ios);
bool equal_alists(void* px, void* py);
int  compare_alists(value_t x, value_t y);
int  init_alist(void* ptr, void* dat);
void trace_alist(void* ptr);
void free_alist(void* ptr);

data_type_t AlistType = {
  .type={
    .obj={
      .type  =DATA_TYPE,
      .frozen=true,
      .gray  =true,
    },
    .name="alist",
    .idno=ALIST
  },
  .size   =sizeof(alist_t),
  .print  =print_alist,
  .equal  =equal_alists,
  .compare=compare_alists,
  .init   =init_alist,
  .trace  =trace_alist,
  .free   =free_alist
};

// utilities ------------------------------------------------------------------
void resize_alist(alist_t* self, usize cnt) {
  usize newc = pad_alist_size(cnt, self->count, self->cap, MIN_CAP);

  if (newc != self->cap) {
    self->array = reallocate(self->array, newc * sizeof(value_t), self->cap * sizeof(value_t));
    self->cap   = newc;
  }
}

// sacred methods -------------------------------------------------------------
void print_alist(value_t x, port_t* ios) {
  alist_t* alist = as_alist(x);

  rl_puts("#alist(", ios);
  print_values(alist->count, alist->array, ios, false);
  rl_putc(')', ios);
}

bool equal_alists(void* px, void* py) {
  alist_t* ax = px, * ay = py;

  return equal_values(ax->count, ax->array, ay->count, ay->array);
}

int compare_alists(value_t x, value_t y) {
  alist_t* ax = as_alist(x), * ay = as_alist(y);

  return equal_values(ax->count, ax->array, ay->count, ay->array);
}

// lifetime -------------------------------------------------------------------
int init_alist(void* ptr, void* dat) {
  (void)dat;

  alist_t* ax = ptr;
  ax->count   = 0;
  ax->cap     = pad_alist_size(0, 0, 0, 8);
  ax->array   = allocate(ax->cap * sizeof(value_t));

  return 0;
}

void trace_alist(void* ptr) {
  alist_t* alist = ptr;

  mark_values(alist->count, alist->array);
}

void free_alist(void* ptr) {
  alist_t* self = ptr;

  deallocate(self->array, self->cap * sizeof(value_t));
}

// misc ----------------------------------------------------------------------------
void reset_alist(alist_t* self) {
  free_alist(self);
  init_alist(self, NULL);
}

usize alist_push(alist_t* self, value_t val) {
  usize out = self->count++;
  resize_alist(self, self->count);
  self->array[out] = val;
  return out;
}

value_t alist_pop(alist_t* self) {
  assert(self->count);

  value_t out = self->array[--self->count];
  resize_alist(self, self->count);

  return out;
}

// initialization -------------------------------------------------------------
void object_init(void) {
  rl_hash(&EmptyList.obj);
}
