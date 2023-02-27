#include "eval.h"



// globals --------------------------------------------------------------------
value_t Stack[N_STACK], *Sp, *Se;


// local helpers --------------------------------------------------------------
bool is_literal(value_t x) {
  if (is_list(x))
    return as_list(x)->len > 0;

  if (is_symbol(x))
    return as_symbol(x)->name[0] == ':';

  return true;
}

usize evargs(list_t* args, bool noev) {
  usize n = 0;

  while (args->len) {
    value_t x = args->head;
    value_t v = noev ? x : eval(x);
    push(v);
    n++;
  }

  return n;
}

// API ------------------------------------------------------------------------
// stack ----------------------------------------------------------------------
value_t* peep(int i) {
  value_t* out = (i < 0 ? Sp : Stack) + i;
  assert(out >= Stack && out < Se);
  return out;
}

value_t peek(int i) {
  return *peep(i);
}

value_t* push(value_t x) {
  assert(Sp < Se);
  value_t* out = Sp++;
  *out = x;
  return out;
}

value_t pop(void) {
  assert(Sp > Stack);
  return *(--Sp);
}

value_t* pushn(usize n) {
  assert(Sp+n < Se);
  value_t* out = Sp;
  Sp += n;
  return out;
}

value_t popn(usize n) {
  value_t out = Sp[-1];
  assert(Sp-n >= Stack);
  Sp -= n;
  return out;
}

// interpreter ----------------------------------------------------------------
value_t eval(value_t x) {
  if (is_literal(x))
    return x;

  if (is_symbol(x))
    return as_symbol(x)->bind;

  // TODO: apply
  return apply(eval(as_list(x)->head), as_list(x)->tail, false);
}

value_t apply(value_t x, list_t* args, bool noev) {
  usize n = evargs(args, noev);
  value_t out = invoke(x, n, peep(-n));
  popn(n);
  return out;
}

value_t invoke(value_t f, usize n, value_t* args);
void    repl(void);
