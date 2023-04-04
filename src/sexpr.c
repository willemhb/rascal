#include <string.h>

#include "sexpr.h"
#include "text.h"

#include "memory.h"
#include "hashing.h"
#include "number.h"

// symbol ---------------------------------------------------------------------
uint64 SymbolCounter = 1;
symbol_t* SymbolTable = NULL;

void  print_symbol(value_t val, port_t* ios);
uhash hash_symbol(void* ptr);
int   compare_symbols(value_t x, value_t y);

void* alloc_symbol(data_type_t* type, usize n, flags fl);
void  init_symbol(void* self, data_type_t* type, usize n, flags fl);
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
};

// sacred methods -------------------------------------------------------------
void print_symbol(value_t val, port_t* ios) {
  symbol_t* s = as_symbol(val);

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

  symbol_t* sx = as_symbol(x), * sy = as_symbol(y);

  if ((o=strcmp(sx->name, sy->name)))
    return o;

  if ((o=CMP(sx->idno, sy->idno)))
    return o;

  return 0;
}

// lifetime methods -----------------------------------------------------------
void* alloc_symbol(data_type_t* type, usize n, flags fl) {
  (void)fl;
  
  return allocate(type->size + n + 1);
}

void init_symbol(void* self, data_type_t* type, usize n, flags fl) {
  (void)type;
  (void)n;
  (void)fl;

  symbol_t* sym = self;

  sym->idno = SymbolCounter++;
  sym->name = (char*)self + sizeof(symbol_t);
  sym->left = sym->right = NULL;
  sym->toplevel = NULL;
}

void mark_symbol(void* self) {
  symbol_t* sym = self;

  mark_object(sym->left);
  mark_object(sym->right);
  mark_object(sym->toplevel);
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

static symbol_t *new_symbol(char* name, flags fl) {
  symbol_t *sym = new_object(&SymbolType, strlen(name), fl);
  strcpy(sym->name, name);
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
      *node = new_symbol(name, INTERNED);

    return *node;
  }

  else
    return new_symbol(name, 0);
}

// list -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
list_t EmptyList = {
  .obj={
    .type  =LIST,
    .frozen=true,
    .gray  =true
  },
  .head =NUL,
  .tail =&EmptyList,
  .arity=0
};

void    print_list(value_t x, port_t* ios);
int     equal_lists(void* x, void* y);
void    trace_list(void* self);
void*   list_iter(void* iterable);
value_t list_next(void** iterbuf);
bool    list_hasnext(void* iterable);

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
  .size   =sizeof(list_t),

  .print  =print_list,
  .equal  =equal_lists,

  .iter   =list_iter,
  .next   =list_next,
  .hasnext=list_hasnext,

  .trace  =trace_list
};

// sacred methods -------------------------------------------------------------
void print_list(value_t val, port_t* ios) {
  rl_putc('(', ios);

  void* xs = as_list(val);

  for (value_t x=next(&xs); x != STOPITER; x=next(&xs)) {
    rl_print(x, ios);

    if (hasnext(xs))
      rl_putc(' ', ios);
  }

  rl_putc(')', ios);
}

int equal_lists(void* px, void* py) {
  list_t* lx = px,* ly = py;

  if (lx->arity != ly->arity)
    return 0;

  return -1;
}

// traversal methods ----------------------------------------------------------
void* list_iter(void* iterable) {
  list_t* xs = iterable;

  return xs->arity ? xs : NULL;
}

value_t list_next(void** iterbuf) {
  list_t* iter = *iterbuf;

  if (iter == &EmptyList)
    return STOPITER;

  value_t out = iter->head;
  *iterbuf = iter->tail;

  return out;
}

bool list_hasnext(void* iterable) {
  return iterable && iterable != &EmptyList;
}

// lifetime methods -----------------------------------------------------------
void trace_list(void* ptr) {
  list_t* list = ptr;

  mark_value(list->head);
  mark_object(list->tail);
}

// ctors ----------------------------------------------------------------------
static list_t* new_list(void) {
  return new_object(&ListType, 0, 0);
}

static void init_list(list_t* xs, value_t head, list_t* tail) {
  init_object(xs, &ListType, 0, 0);

  xs->obj.frozen = true;
  xs->head       = head;
  xs->tail       = tail;
  xs->arity      = tail->arity+1;
}

list_t* cons(value_t head, list_t* tail) {
  list_t* out = new_list();

  init_list(out, head, tail);

  return out;
}

list_t* list(usize n, value_t* args) {
  if (n == 0)
    return &EmptyList;

  if (n == 1)
    return cons(args[0], &EmptyList);

  list_t* out  = allocate(n * sizeof(list_t));
  list_t* curr = &out[n-1], *last = &EmptyList;

  for (usize i=n; i>0; i--, last=curr--)
    init_list(curr, args[n-1], last);

  return out;
}

// initialization -------------------------------------------------------------
void object_init(void) {
  rl_hash(&EmptyList.obj);
}
