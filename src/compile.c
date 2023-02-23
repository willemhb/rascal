#include <stdarg.h>

#include "object.h"
#include "compile.h"
#include "runtime.h"

// C types --------------------------------------------------------------------
// globals --------------------------------------------------------------------
Val Quote, Do, Var, Put, Lmb, If;

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
void   init_chunk(Chunk* chunk, Vec* vals, Bin* code, List* lenv, List* cenv);

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
List* build_lenv(List* formals, List* parent, flags* fl, uint* arity);

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
uint compile_defv(Chunk* c, List* form);
uint compile_setv(Chunk* c, List* form);
uint compile_fn(Chunk* c, List* form);
uint compile_if(Chunk* c, List* form);
uint compile_funcall(Chunk* c, List* form);

// miscellaneous compile dispatch ---------------------------------------------
uint compile_funargs(Chunk* c, List* xs);
uint compile_sequence(Chunk* c, List* xs);

// chunk API ------------------------------------------------------------------
void init_chunk(Chunk* ch, Vec* vals, Bin* code, List* lenv, List* cenv) {
  if (vals == NULL)
    vals = new_vec(0, NULL);

  if (code == NULL)
    code = new_bin(0, 0, NULL);

  if (lenv == NULL)
    lenv = &EmptyList;

  ch->vals = vals;
  ch->code = code;
  ch->lenv = lenv;
  ch->cenv = cenv;
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
  case OP_DEF_LOCAL ... OP_SET_LOCAL:
  case OP_JUMP ... OP_JUMP_IF_FALSE:
  case OP_INVOKE_FUNCTION ... OP_TAIL_INVOKE_FUNCTION:
    return 1;

  case OP_SET_CLOSURE:
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

List* build_lenv(List* formals, List* parent, flags* fl, uint* arity) {
  Vec* locals = new_vec(0, NULL);

  bool rest_argument = false;

  for (;formals->arity; formals = formals->tail) {
    Val name = formals->head;

    GUARD_STX(has_type(name, SYM),
	  &EmptyList,
	  "formal argument is not a symbol" );

    if (name == Vargs || name == Vbody) {
      GUARD_STX(!flagp(*fl, VARGS),
		&EmptyList,
		"bad syntax in fn: rest syntax marker appears twice" );

      GUARD_STX(formals->arity > 1,
                &EmptyList,
                "bad syntax in fn: rest syntax marker but no rest formal" );

      *fl |= VARGS;
      rest_argument = true;
    } else if (name == Vopts) {
      GUARD_STX(!flagp(*fl, VOPTS),
		&EmptyList,
		"bad syntax in fn: options syntax marker appears twice" );

      GUARD_STX(formals->arity > 1,
                &EmptyList,
                "bad syntax in fn: options syntax marker but no options formal" );

      *fl |= VOPTS;
      rest_argument = true;
    } else if (name == Vkwargs) {
      GUARD_STX(!flagp(*fl, VKWARGS),
		&EmptyList,
		"bad syntax in fn: keywords syntax marker appears twice" );

      GUARD_STX(formals->arity > 1,
                &EmptyList,
                "bad syntax in fn: keywords syntax marker but no keywords formal" );

      *fl |= VKWARGS;
      rest_argument = true;
    } else {
      uint n = locals->count;

      GUARD_STX(add_to_vec(locals, name) == n,
		&EmptyList,
		"bad syntax in fn: formal argument '%s' appears twice",
		as_sym(name)->name );

      if (!rest_argument)
        (*arity)++;

      rest_argument = false;
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

  if (form->head == Var)
    return compile_defv(c, form);

  if (form->head == Put)
    return compile_setv(c, form);

  if (form->head == Lmb)
    return compile_fn(c, form);

  if (form->head == If)
    return compile_if(c, form);

  return compile_funcall(c, form);
}

uint compile_quote(Chunk* c, List* form) {
  GUARD_STX( form->arity==2, 0, "bad syntax in quote: %du expressions", form->arity);

  Val x = form->tail->head;

  return compile_literal(c, x);
}

uint compile_do(Chunk* c, List* form) {
  GUARD_STX(form->arity >= 2, 0, "bad syntax in do: %du expressions", form->arity);

  return compile_sequence(c, form->tail);
}

uint compile_defv(Chunk* c, List* form) {
  GUARD_STX(form->arity == 3, 0, "bad syntax in defv: %du expressions", form->arity);

  Val name = list_nth(form, 1);
  Val bind = list_nth(form, 2);

  GUARD_STX(has_type(name, SYM), 0, "bad syntax in defv: name is not a symbol");

  int i, j;

  OpCode assign;

  define_name(name, c, &i, &j);

  if (i == -1) {
    assign = OP_SET_GLOBAL;
    emit(c, OP_DEF_GLOBAL, j);
  } else {
    emit(c, OP_DEF_LOCAL, j);
    assign = OP_SET_LOCAL;
  }

  compile_expr(c, bind);
  REPANIC(0);

  return emit(c, assign, j);
}

uint compile_setv(Chunk* c, List* form) {
    GUARD_STX(form->arity == 3, 0, "bad syntax in setv: %du expressions", form->arity);

  Val name = list_nth(form, 1);
  Val bind = list_nth(form, 2);

  GUARD_STX(has_type(name, SYM), 0, "bad syntax in setv: name is not a symbol");

  int i, j;
    
  OpCode assign;

  resolve_name(name, c, &i, &j);


  if (i == -1)
    assign = OP_SET_GLOBAL;

  else if (i == 0)
    assign = OP_SET_LOCAL;

  else
    assign = OP_SET_CLOSURE;

  compile_expr(c, bind);
  REPANIC(0);

  return emit(c, assign, j);
}

uint compile_fn(Chunk* c, List* form) {
  GUARD_STX(form->arity >= 3,
            0,
            "bad syntax in fn: %du expressions",
            form->arity );

  Val formals = list_nth(form, 1);
  List* body  = list_tail(form, 2);

  GUARD_STX(has_type(formals, LIST),
            0,
            "bad syntax in fn: formals is not a list" );

  uint arity = 0;
  flags fl   = USER|CLOSURE;
  List* lenv = build_lenv(as_list(formals), c->lenv, &fl, &arity);

  REPANIC(0);

  Chunk lambda;

  init_chunk(&lambda, NULL, NULL, lenv, NULL);
  compile_sequence(&lambda, body);

  REPANIC(0);

  emit(&lambda, OP_RETURN);

  Func* f = new_func(fl, arity, Lambda, NULL, &lambda);

  compile_literal(c, tag(f));

  return emit(c, OP_CAPTURE_CLOSURE);
}

uint compile_if(Chunk* c, List* form) {
  GUARD_STX(form->arity == 3 || form->arity == 4,
            0,
            "bad syntax in if: %du expressions",
            form->arity );

  Val test = list_nth(form, 1);
  Val then = list_nth(form, 2);
  Val alt  = form->arity == 3 ? NUL : list_nth(form, 3);

  uint off1, off2, off3;

  compile_expr(c, test);
  REPANIC(0);

  off1 = emit(c, OP_JUMP_IF_FALSE, 0) - 1;

  compile_expr(c, then);
  REPANIC(0);

  off2 = emit(c, OP_JUMP, 0) - 1;

  compile_expr(c, alt);
  REPANIC(0);

  off3 = code_size(c->code);

  code_set(c->code, off1, off2-off1);
  code_set(c->code, off2, off3-off2);

  return code_size(c->code);
}

uint compile_funcall(Chunk* c, List* form) {
  Val   head = form->head;
  List* args = form->tail;
  uint  argc = args->arity;

  compile_expr(c, head);
  REPANIC(0);

  compile_funargs(c, args);
  REPANIC(0);

  return emit(c, OP_INVOKE_FUNCTION, argc);
}

// misc compile dispatch ------------------------------------------------------
uint compile_funargs(Chunk* c, List* xs) {
  for (Val x=xs->head; xs != &EmptyList; xs=xs->tail, x=xs->head) {
    compile_expr(c, x);
    REPANIC(0);
  }

  return code_size(c->code);
}

uint compile_sequence(Chunk* c, List* xs) {
  for (Val x=xs->head;xs != &EmptyList; xs=xs->tail, x=xs->head) {
    compile_expr(c, x);
    REPANIC(0);

    if (xs->arity > 1)
      emit(c, OP_POP);
  }

  return code_size(c->code);
}

// API ------------------------------------------------------------------------
Func* compile(Val x) {
  Chunk c;

  init_chunk(&c, NULL, NULL, NULL, NULL);

  emit(&c, OP_BEGIN);
  compile_expr(&c, x);
  REPANIC(NULL);
  emit(&c, OP_RETURN);

  Func* out = new_func(TOPLEVEL, 0, Toplevel, NULL, &c);

  FNCENV(out) = &EmptyList;

  return out;
}

// initialization -------------------------------------------------------------
void compile_init(void) {
  // special form names -------------------------------------------------------
  Quote    = tag(get_sym(LITERAL, "quote"));
  Do       = tag(get_sym(LITERAL, "do"));
  Var      = tag(get_sym(LITERAL, "var"));
  Put      = tag(get_sym(LITERAL, "put"));
  Lmb      = tag(get_sym(LITERAL, "lmb"));
  If       = tag(get_sym(LITERAL, "if"));

  // special syntactic markers ------------------------------------------------
  Vargs    = tag(get_sym(LITERAL, "&va"));
  Vopts    = tag(get_sym(LITERAL, "&opt"));
  Vkwargs  = tag(get_sym(LITERAL, "&kw"));

  // dummy function names -----------------------------------------------------
  Toplevel = get_sym(0, "*toplevel*");
  Lambda   = get_sym(0, "*lambda*");
}
