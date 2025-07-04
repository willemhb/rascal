/* DESCRIPTION */
// headers --------------------------------------------------------------------

#include "data/list.h"

#include "lang/io.h"
#include "lang/print.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_list(Port* ios, Expr x);
bool egal_lists(Expr x, Expr y);
void trace_list(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_list(Port* ios, Expr x) {
        pprintf(ios, "(");

      List* xs = as_list(x);

      while ( xs->count > 0 ) {
        print_exp(ios, xs->head);

        if ( xs->count > 1 )
          pprintf(ios, " ");

        xs = xs->tail;
      }

      pprintf(ios, ")");
}

bool egal_lists(Expr x, Expr y) {
  List* xs = as_list(x), * ys = as_list(y);

  bool out = xs->count == ys->count;

  while ( out && xs->count > 0 ) {
    x   = xs->head;
    y   = ys->head;
    out = egal_exps(x, y);

    if ( out ) {
      xs = xs->tail;
      ys = ys->tail;
    }
  }

  return out;
}

void trace_list(void* ptr) {
  List* xs = ptr;

  if ( xs->count ) {
    mark_exp(xs->head);
    mark_obj(xs->tail);
  }
}

// external -------------------------------------------------------------------
static List* new_lists(size_t n) {
  // TODO: significantly alter this, as written it introduces potential UB.
  assert(n > 0);

  size_t nb = (n+1) * sizeof(List);
  List* xs  = allocate(true, nb);

  // initialize terminal empty list
  for ( size_t i=0; i < n; i++ ) {
    List* cell = &xs[i];

    // initialize the list object
    cell->heap   = (Obj*)(&cell+1);
    cell->type   = EXP_LIST;
    cell->black  = false;
    cell->gray   = true;
    cell->head   = NUL;
    cell->tail   = cell + 1;
    cell->count  = n - i;
  }

  // handle the terminal empty list specially
  List* cell  = &xs[n];
  cell->heap   = Heap;
  cell->type   = EXP_LIST;
  cell->black  = false;
  cell->gray   = true;
  cell->head   = NUL;
  cell->tail   = NULL;
  cell->count  = 0;

  // add it all to the heap
  Heap = (Obj*)xs;

  return xs;
}

List* as_list_s(char* f, Expr x) {
  require_argtype(f, EXP_LIST, x);

  return as_list(x);
}

List* empty_list(void) {
  List* l = mk_obj(EXP_LIST, 0);

  l->head  = NUL;
  l->tail  = NULL;
  l->count = 0;

  return l;
}

List* mk_list(size_t n, Expr* xs) {
  List* l;

  if ( n == 0 )
    l = empty_list();

  else {
    l = new_lists(n);

    for ( size_t i=0; i<n; i++ )
      l[i].head = xs[i];
  }

  return l;
}

List* cons(Expr hd, List* tl) {
  assert(tl != NULL);
  preserve(1, tag_obj(tl));

  List* l  = mk_obj(EXP_LIST, 0);
  l->head  = hd;
  l->tail  = tl;
  l->count = tl->count+1;

  return l;
}

Expr list_ref(List* xs, int n) {
  assert(n >= 0);
  assert(n < (int)xs->count-1);

  while ( n-- )
    xs = xs->tail;

  return xs->head;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_list(void) {
  Types[EXP_LIST] = (ExpTypeInfo) {
    .type     = EXP_LIST,
    .name     = "list",
    .obsize   = sizeof(List),
    .print_fn = print_list,
    .egal_fn  = egal_lists,
    .trace_fn = trace_list     
  };
}
