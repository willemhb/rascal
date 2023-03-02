#include <stdio.h>

#include "error.h"
#include "eval.h"
#include "io.h"


// globals --------------------------------------------------------------------
value_t Stack[N_STACK], *Sp, *Se;

value_t Quote, Put, If, Do;

// local helpers --------------------------------------------------------------
bool is_literal(value_t x) {
  if (is_list(x))
    return as_list(x)->len == 0;

  if (is_symbol(x))
    return as_symbol(x)->name[0] == ':';

  return true;
}

usize evargs(list_t* args, bool noev) {
  usize n = 0;

  while (args->len) {
    if (panicking()) {
      popn(n);
      return 0;
    }

    value_t x = args->head;
    value_t v = noev ? x : eval(x);
    push(v);
    n++;
    args = args->tail;
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
// helpers --------------------------------------------------------------------
value_t ev_sequence(list_t* form) {
  value_t v = NUL;
  
  while (form->len) {
    v    = eval(form->head);
    form = form->tail;
  }

  return v;
}

value_t is_form(value_t h, list_t* form) {
  return form->head == h;
}

value_t is_quote(list_t* form) {
  return is_form(Quote, form);
}

value_t is_put(list_t* form) {
  return is_form(Put, form);
}

value_t is_if(list_t* form) {
  return is_form(If, form);
}

value_t is_do(list_t* form) {
  return is_form(Do, form);
}

value_t ev_quote(list_t* form) {
  stxarity(1, form->len-1, "quote");

  return nth_hd(form, 1);
}

value_t ev_put(list_t* form) {

  stxarity(2, form->len-1, "put");
  value_t name = nth_hd(form, 1);
  value_t val  = nth_hd(form, 2);
  
  stxtype(SYMBOL, name, "put");

  value_t bind = eval(val);
  as_symbol(name)->bind = bind;

  return bind;
}

value_t ev_if(list_t* form) {
  stxvarity(2, form->len-1, "if");

  list_t* tests = form->tail;

  value_t test = NUL;

  while (tests->len >= 2) {
    test = eval(form->head);

    if (test != NUL && test != FALSE_VAL)
      return eval(tests->tail->head);

    tests = tests->tail->tail;
  }

  if (tests->len)
    return eval(tests->head);

  return test;
}

value_t ev_do(list_t* form) {
  stxvarity(1, form->len-1, "do");
  return ev_sequence(form->tail);
}

value_t eval(value_t x) {
  repanic(x);

  if (is_literal(x))
    return x;

  if (is_symbol(x)) {
    require(as_symbol(x)->bind != UNBOUND,
	    EVAL_ERROR,
	    NUL,
	    "unbound symbol '%s",
	    as_symbol(x)->name);
 
    return as_symbol(x)->bind;
  }

  list_t* form = as_list(x);

  if (is_quote(form))
    return ev_quote(form);

  if (is_put(form))
    return ev_put(form);

  if (is_if(form))
    return ev_if(form);

  if (is_do(form))
    return ev_do(form);

  // TODO: apply
  value_t h = form->head;
  value_t f = eval(h); repanic(NUL);
  list_t* a = as_list(x)->tail;

  argtest(is_function, f, "can't apply non-function");

  return apply(f, a, false);
}

value_t apply(value_t x, list_t* args, bool noev) {
  assert(is_function(x));
  usize n = evargs(args, noev);
  repanic(NUL);
  value_t out = invoke(x, n, peep(-n));
  popn(n);
  return out;
}

value_t invoke(value_t f, usize n, value_t* args) {
  value_t out;
  assert(is_function(f));
  out = as_native(f)->func(n, args);
  return out;
}

#define PROMPT "rascal>"

void repl(void) {
  value_t a;

  for (;;) {
    printf(PROMPT" ");
    value_t x = read();
    value_t v = recover(&a) ? a : eval(x);

    if (recover(&a))
      v = a;

#ifdef RASCAL_DEBUG
    printf("%s> ", type_name_of(v));
#endif
    
    print(v);
    printf("\n");
  }
}

// initialization -------------------------------------------------------------
void eval_init(void) {
  Quote = symbol("quote");
  Put   = symbol("put");
  If    = symbol("if");
  Do    = symbol("do");
  Sp    = Stack;
  Se    = Stack+N_STACK;
}
