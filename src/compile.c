#include <stdarg.h>

#include "object.h"
#include "compile.h"
#include "runtime.h"

// C types --------------------------------------------------------------------
// globals --------------------------------------------------------------------
Val Quote, Do, Def, Set, Fn, If;

Val Vargs, Vopts, Vkwargs;

Sym* Toplevel, * Lambda;

// local helpers --------------------------------------------------------------
#define FNCHUNK(fn) ((Chunk*)((fn)->func))
#define FNVALS(fn)  (FNCHUNK(fn)->vals)
#define FNCODE(fn)  (FNCHUNK(fn)->code)
#define FNLENV(fn)  (FNCHUNK(fn)->lenv)
#define FNCENV(fn)  (FNCHUNK(fn)->cenv)

#define GUARD_STX(test, sentinel, fmt, ...)				\
  GUARD(test, sentinel, COMPILE_ERROR, fmt __VA_OPT__(,) __VA_ARGS__)

// chunk API ------------------------------------------------------------------
Chunk* mk_chunk(void);
void   init_chunk(Chunk* chunk);

// general utility ------------------------------------------------------------
uint add_to_vec(Vec* vec, Val x);

// opcodes & bytecode ---------------------------------------------------------
uint   code_size(Bin* bin);
uint16 code_get(Bin* bin, uint n);
uint16 code_set(Bin* bin, uint n, uint16 byte);
uint   code_write(Bin* bin, uint n, uint16* byte);
uint   op_argc(OpCode op);
uint   emit(Chunk* c, OpCode op, ...);

// environments ---------------------------------------------------------------
bool is_toplevel(Chunk* c);
void resolve_name(Val n, Chunk* c, int* i, int* j);
void define_name(Val n, Chunk* c, int* i, int* j);
List* build_lenv(List* formals, List* parent, flags* fl);

// misc -----------------------------------------------------------------------
bool   is_literal(Val x);
uint   add_const(Chunk* c, Val x);

// compile dispatch -----------------------------------------------------------
uint compile_expr(Chunk* c, Val x);
uint compile_literal(Chunk* c, Val x);
uint compile_reference(Chunk* c, Val x);
uint compile_application(Chunk* c, Val x);

// compile application dispatch -----------------------------------------------
uint compile_quote(Chunk* c, List* form);
uint compile_do(Chunk* c, List* form);
uint compile_def(Chunk* c, List* form);
uint compile_set(Chunk* c, List* form);
uint compile_fn(Chunk* c, List* form);
uint compile_if(Chunk* c, List* form);
uint compile_funcall(Chunk* c, List* form);

// miscellaneous compile dispatch ---------------------------------------------
uint compile_funargs(Chunk* c, List* xs);
uint compile_sequence(Chunk* c, List* xs);

// chunk API ------------------------------------------------------------------
Chunk* new_chunk(Vec* vals, Bin* code, List* lenv, List* cenv) {
  
}

// misc -----------------------------------------------------------------------
uint add_to_vec(Vec* vec, Val x) {
  assert(vec != NULL);

  int i = vec_search(vec, x);

  if (i < 0)
    i = vec_push(vec, x);

  return i;
}

bool is_literal(Val x) {
  if (has_type(x, SYM))
    return !has_flag(as_sym(x), LITERAL);

  if (has_type(x, LIST))
    return as_list(x) != &EmptyList;

  return true;
}

// opcodes & bytecode ---------------------------------------------------------
uint code_size(Bin* bin) {
  return bin->count >> 1;
}

uint16 code_get(Bin* bin, uint n) {
  return ((uint16*)bin->array)[n];
}

uint16 code_set(Bin* bin, uint n, uint16 byte) {
  ((uint16*)bin->array)[n] = byte;

  return byte;
}

uint code_write(Bin* bin, uint n, uint16* bytes) {
  uint out = bin_write(bin, n<<1, bytes);

  return out>>1;
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

uint emit(Chunk* c, OpCode op, ...) {
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

  return code_write(c->code, n, buf);
}

uint add_const(Chunk* c, Val x) {
  return add_to_vec(c->vals, x);
}

// environment and variable helpers -------------------------------------------
bool is_toplevel(Chunk* c) {
  return c->lenv == &EmptyList;
}

void resolve_name(Val n, Chunk* c, int* i, int* j) {
  List* lenv = c->lenv;

  *i = 0;
  *j = 0;

  while (lenv != &EmptyList) {
    *j = vec_search(as_vec(lenv->head), n);

    if (*j == -1)
      (*i)++;

    else
      return;
  }

  *i = -1;              // signal that local value was not found
  *j = add_const(c, n); // save the location of the key to be referenced at runtime
}

void define_name(Val n, Chunk* c, int* i, int* j) {
  *i = 0;
  *j = 0;

  List* e = c->lenv;

  if (e == &EmptyList) {
    *i = -1;
    *j = add_const(c, n);
  } else {
    *j = add_to_vec(as_vec(e->head), n);
  }
}

List* build_lenv(List* formals, List* parent, flags* fl) {
  Vec* locals = new_vec(0, NULL);

  for (;formals->arity; formals = formals->tail) {
    Val name = formals->head;

    GUARD_STX(has_type(name, SYM),
	  &EmptyList,
	  "formal argument is not a symbol" );

    if (name == Vargs) {
      GUARD_STX(!flagp(*fl, VARGS),
		&EmptyList,
		"bad syntax in fn: rest syntax marker appears twice" );

      *fl |= VARGS;
    } else if (name == Vopts) {
      GUARD_STX(!flagp(*fl, VOPTS),
		&EmptyList,
		"bad syntax in fn: options syntax marker appears twice" );

      *fl |= VOPTS;
    } else if (name == Vkwargs) {
      GUARD_STX(!flagp(*fl, VKWARGS),
		&EmptyList,
		"bad syntax in fn: keywords syntax marker appears twice" );

      *fl |= VKWARGS;
    } else {
      uint n = locals->count;

      GUARD_STX(add_to_vec(locals, name) == n,
		&EmptyList,
		"bad syntax in fn: formal argument '%s' appears twice",
		as_sym(name)->name );
    }
  }

  return new_list(tag(locals), parent);
}

// compile dispatch -----------------------------------------------------------
uint compile_expr(Chunk* c, Val x) {
  if (is_literal(x))
    return compile_literal(c, x);

  else if (has_type(x, LIST))
    return compile_application(c, x);

  return compile_reference(c, x);
}

uint compile_literal(Chunk* c, Val x) {
  if (x == NUL)
    return emit(c, OP_LOAD_NUL);

  uint arg = add_const(c, x);

  return emit(c, OP_LOAD_CONST, arg);
}

uint compile_reference(Chunk* c, Val x) {
  int i, j;

  resolve_name(x, c, &i, &j);

  if (i == 0)
    return emit(c, OP_LOAD_LOCAL, j);

  else if (i == -1)
    return emit(c, OP_LOAD_GLOBAL, j);

  else
    return emit(c, OP_LOAD_CLOSURE, i, j);
}

uint compile_application(Chunk* c, Val x) {
  List* form = as_list(x);

  if (form->head == Quote)
    return compile_quote(c, form);

  if (form->head == Do)
    return compile_do(c, form);

  if (form->head == Def)
    return compile_def(c, form);

  if (form->head == Set)
    return compile_set(c, form);

  if (form->head == Fn)
    return compile_fn(c, form);

  if (form->head == If)
    return compile_if(c, form);

  return compile_funcall(c, form);
}

uint compile_quote(Chunk* c, List* form) {
  GUARD( form->arity==2,
	 0,
	 COMPILE_ERROR,
	 "bad syntax in quote: %du expressions", form->arity );

  Val x = form->tail->head;

  return compile_literal(c, x);
}

uint compile_do(Chunk* c, List* form) {
  GUARD(form->arity >= 2,
	0,
	COMPILE_ERROR,
	"bad syntax in do: %du expressions", form->arity );

  return compile_sequence(c, form->tail);
}

// misc compile dispatch ------------------------------------------------------
uint compile_funargs(Chunk* c, List* xs) {
  for (;xs != &EmptyList; xs=xs->tail) {
    Val arg = xs->head;
    compile_expr(c, arg);
    REPANIC(0);
  }

  return code_size(c->code);
}

// API ------------------------------------------------------------------------
Func* compile(Val x) {
  Chunk* c = new_chunk(NULL, NULL, NULL, NULL);

  emit(, OP_BEGIN);
  compile_expr(target, x);
  emit(target, OP_RETURN);

  return target;
}

// initialization -------------------------------------------------------------
void compile_init(void) {
  // special form names -------------------------------------------------------
  Quote  = tag(get_sym(LITERAL, "quote"));
  Do     = tag(get_sym(LITERAL, "do"));
  Def    = tag(get_sym(LITERAL, "def"));
  Set    = tag(get_sym(LITERAL, "set"));
  Fn     = tag(get_sym(LITERAL, "fn"));
  If     = tag(get_sym(LITERAL, "if"));

  // special syntactic markers ------------------------------------------------
  Vargs  = tag(get_sym(LITERAL, "&va"));
  Vopts  = tag(get_sym(LITERAL, "&opt"));
  Vkwargs= tag(get_sym(LITERAL, "&kw"));

  // dummy function names -----------------------------------------------------
  Toplevel     =get_sym(0, "*toplevel*");
  Lambda       =get_sym(0, "*lambda*");
}
