#include "sexpr.h"



// symbol ---------------------------------------------------------------------
uint64 SymbolCounter = 1;
symbol_t* SymbolTable = NULL;


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

// initialization -------------------------------------------------------------
void object_init(void) {
  rl_hash(&EmptyList.obj);
}
