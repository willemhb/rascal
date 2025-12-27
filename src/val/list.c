#include "val/list.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"

// forward declarations
void print_list(Port* ios, Expr x);
bool egal_lists(Expr x, Expr y);
void trace_list(RlState* rls, void* ptr);

// Type object
Type ListType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_LIST,
  .obsize   = sizeof(List),
  .print_fn = print_list,
  .egal_fn  = egal_lists,
  .trace_fn = trace_list
};

// list API
static void init_list(List* lx, Expr hd, List* tl) {
  lx->head = hd;
  lx->tail = tl;
  lx->count = tl ? tl->count+1 : 0;
  lx->line = -1;
}

List* empty_list(RlState* rls) {
  List* l = mk_obj(rls, &ListType, 0);
  init_list(l, NUL, NULL);
  return l;
}

List* empty_list_s(RlState* rls) {
  List* out = empty_list(rls);
  stack_push(rls, tag_obj(out));
  return out;
}

List* mk_list(RlState* rls, int n) {
  StackRef top = rls->s_top;
  List* l = empty_list(rls);
  StackRef xs = rls->s_top-n;

  if ( n > 0 ) {
    stack_push(rls, tag_obj(l));

    for ( size_t i=n; i>0; i-- ) {
      l = cons(rls, xs[i-1], l);
      tos(rls) = tag_obj(l);
    }
  }

  rls->s_top = top;
  return l;
}

List* mk_list_s(RlState* rls, int n) {
  List* out = mk_list(rls, n);
  stack_push(rls, tag_obj(out));
  return out;
}

List* cons(RlState* rls, Expr hd, List* tl) {
  assert(tl != NULL);
  StackRef top = rls->s_top;
  stack_push(rls, tag_obj(tl));
  List* l = mk_obj(rls, &ListType, 0);
  init_list(l, hd, tl);
  rls->s_top = top;
  return l;
}

List* cons_s(RlState* rls, Expr hd, List* tl) {
  List* out = cons(rls, hd, tl);
  stack_push(rls, tag_obj(out));
  return out;
}

List* cons_n(RlState* rls, int n) {
  assert(n >= 2);
  StackRef top = rls->s_top;
  Expr* xs = rls->s_top-n;
  List* lx, * ly;
  assert(is_list(xs[n-1]));

  if ( n == 2 ) {
    ly = as_list(xs[n-1]);
    lx = cons(rls, xs[0], ly);
  } else {
    Expr* buf = stack_dup(rls);

    for ( int i=n-2; i >= 0; i-- ) {
      lx = as_list(*buf);
      ly = mk_obj(rls, &ListType, 0);
      init_list(ly, xs[i], lx);
      *buf = tag_obj(ly);
    }

    lx = as_list(*buf);
  }
  rls->s_top = top;

  return lx;
}

List* cons_n_s(RlState* rls, int n) {
  List* out = cons_n(rls, n);
  stack_push(rls, tag_obj(out));
  return out;
}


Expr list_ref(List* xs, int n) {
  assert(n >= 0);
  assert(n < (int)xs->count-1);

  while ( n-- )
    xs = xs->tail;

  return xs->head;
}

int push_list(RlState* rls, List* xs) {
  int out = xs->count;
  stack_check_limit(rls, out);

  while ( xs->count > 0 ) {
    stack_push(rls, xs->head);
    xs = xs->tail;
  }

  return out;
}

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

void trace_list(RlState* rls, void* ptr) {
  List* xs = ptr;

  if ( xs->count ) {
    mark_exp(rls, xs->head);
    mark_obj(rls, xs->tail);
  }
}
