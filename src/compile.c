#include <stdarg.h>

#include "object.h"
#include "compile.h"
#include "runtime.h"

// C types --------------------------------------------------------------------
// globals --------------------------------------------------------------------
Val Quote, Do, Def, Set, Fn, If;

Sym* Toplevel, * Lambda;

// local helpers --------------------------------------------------------------
#define FNCHUNK(f) ((Chunk*)((f)->func))
#define FNCODE(f)  (FNCHUNK(f)->code)
#define FNLENV(f)  (FNCHUNK(f)->lenv)
#define FNCENV(f)  (FNCHUNK(f)->cenv)
#define FNVALS(f)  (FNCHUNK(f)->vals)

bool is_literal(Val x);
uint op_argc(OpCode op);
uint emit(Func* f, OpCode op, ...);
uint add_const(Func* f, Val x);

void resolve_name(Val n, Func* f, int* i, int* j);
void define_name(Val n, Func* f, int* i, int* j);

uint compile_expr(Func* f, Val x);
uint compile_literal(Func* f, Val x);
uint compile_reference(Func* f, Val x);
uint compile_application(Func* f, Val x);

uint compile_quote(Func* f, List* form);
uint compile_do(Func* f, List* form);
uint compile_def(Func* f, List* form);
uint compile_set(Func* f, List* form);
uint compile_fn(Func* f, List* form);
uint compile_if(Func* f, List* form);
uint compile_funcall(Func* f, List* form);

uint compile_funargs(Func* f, List* xs);
uint compile_sequence(Func* f, List* xs);

bool is_literal(Val x) {
  if (has_type(x, SYM))
    return !has_flag(as_sym(x), LITERAL);

  if (has_type(x, LIST))
    return as_list(x) != &EmptyList;

  return true;
}

uint op_argc(OpCode op) {
  switch (op) {

  case OP_LOAD_CONST ... OP_LOAD_LOCAL:
    return 1;

  case OP_LOAD_CLOSURE:
    return 2;

  default:
    return 0;
  }
}

uint emit(Func* f, OpCode op, ...) {
  uint n = 1;
  uint16 buf[3] = { op, 0, 0 };
  va_list va; va_start(va, op);

  switch (op_argc(op)) {
  case 0:
    break;

  case 1:
    buf[n++] = va_arg(va, int);
    break;
 
  case 2:
    buf[n++] = va_arg(va, int);
    buf[n++] = va_arg(va, int);
    break;
  }

  va_end(va);

  return bin_write(FNCODE(f), n*sizeof(uint16), buf) >> 1;
}

uint add_const(Func* f, Val x) {
  Vec* v = FNVALS(f);

  for (uint i=0; i<v->count; i++)
    if (v->array[i] == x)
      return i;

  return vec_push(v, x);
}

void resolve_name(Val n, Func* f, int* i, int* j) {
  List* e = FNLENV(f);

  *i = 0;
  *j = 0;

  while (e->arity) {
    Table* locals = as_table(e->head);
    Val location  = table_get(locals, n);

    if (location == NOTFOUND) {
      (*i)++;
      e = e->tail;
    } else {
      (*j) = as_int(location);
      return;
    }
  }

  *i = -1;              // signal that local value was not found
  *j = add_const(f, n); // save the location of the key to be referenced at runtime
}

uint compile_expr(Func* f, Val x) {
  if (is_literal(x))
    return compile_literal(f, x);

  else if (has_type(x, LIST))
    return compile_application(f, x);

  return compile_reference(f, x);
}

uint compile_literal(Func* f, Val x) {
  if (x == NUL)
    return emit(f, OP_LOAD_NUL);

  uint arg = add_const(f, x);

  return emit(f, OP_LOAD_CONST, arg);
}

uint compile_reference(Func* f, Val x) {
  int i, j;

  resolve_name(x, f, &i, &j);

  if (i == 0)
    return emit(f, OP_LOAD_LOCAL, j);

  else if (i == -1)
    return emit(f, OP_LOAD_GLOBAL, j);

  else
    return emit(f, OP_LOAD_CLOSURE, i, j);
}

uint compile_application(Func* f, Val x) {
  List* form = as_list(x);

  if (form->head == Quote)
    return compile_quote(f, form);

  if (form->head == Do)
    return compile_do(f, form);

  if (form->head == Def)
    return compile_def(f, form);

  if (form->head == Set)
    return compile_set(f, form);

  if (form->head == Fn)
    return compile_fn(f, form);

  if (form->head == If)
    return compile_if(f, form);

  return compile_funcall(f, form);
}

// API ------------------------------------------------------------------------
Func* compile(Val x) {
  Func* target = new_func(TOPLEVEL, 0, Toplevel, NULL, NULL);

  emit(target, OP_BEGIN);
  compile_expr(target, x);
  emit(target, OP_RETURN);

  return target;
}

// initialization -------------------------------------------------------------
void compile_init(void) {
  // special form names -------------------------------------------------------
  Sym* defsym  =get_sym(false, "def");   set_flag(defsym, LITERAL);   Def   = tag(defsym);
  Sym* setsym  =get_sym(false, "set");   set_flag(setsym, LITERAL);   Set   = tag(setsym);
  Sym* fnsym   =get_sym(false, "fn");    set_flag(fnsym, LITERAL);    Fn    = tag(fnsym);
  Sym* dosym   =get_sym(false, "do");    set_flag(dosym, LITERAL);    Do    = tag(dosym);
  Sym* ifsym   =get_sym(false, "if");    set_flag(ifsym, LITERAL);    If    = tag(ifsym);
  Sym* quotesym=get_sym(false, "quote"); set_flag(quotesym, LITERAL); Quote = tag(quotesym);

  // dummy function names -----------------------------------------------------
  Toplevel     =get_sym(false, "*toplevel*");
  Lambda       =get_sym(false, "*lambda*");
}
