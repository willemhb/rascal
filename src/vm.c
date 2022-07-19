#include <string.h>

#include "vm.h"
#include "function.h"
#include "list.h"
#include "symbol.h"
#include "array.h"
#include "runtime.h"
#include "object.h"
#include "number.h"

static size_t    comp_expression( value_t *e, value_t *v, value_t *c, value_t x );
static size_t    emit_instruction( value_t *code, opcode_t op, short argx, short argy );
static size_t    add_constant( value_t *vals, value_t x );
static opcode_t  resolve_name( value_t *envt, value_t *vals, value_t x, short *i, short *j);
static size_t    comp_sequence( value_t *e, value_t *v, value_t *c, value_t *s, bool acc );
static size_t    comp_closure( value_t *e,value_t *v,value_t *c, value_t *f, value_t *s );

static value_t   ensure_quote( value_t x );
static value_t   ensure_if( value_t x );
static value_t   ensure_do( value_t x );
static value_t   ensure_define( value_t x );
static value_t   ensure_lambda( value_t x );
static value_t   ensure_assign( value_t x );
static value_t   ensure_formals( value_t f, size_t *n, bool *v );

#define bytecode(b)  ((short*)adata(b))
#define elements(v)  ((value_t*)adata(v))
#define r_iseql(x,y) (r_order(x,y)==0)

static size_t vector_find( value_t vec, value_t x ) {
    size_t location = 0;

    for (;location<alength(vec); location++)
      if (r_iseql(x, elements(vec)[location]))
	break;

    return location;
}

static value_t ensure_quote( value_t x ) {
  s_argc( "quote", x, 2 );

  return cadr( x );
}

static value_t ensure_if( value_t x ) {
  s_vargc( "if", x, 3 );

  return cdr( x );
}

static value_t ensure_do( value_t x ) {
  s_vargc( "do", x, 2 );


  return cdr( x );
}

static value_t ensure_define( value_t x ) {
  size_t fl = s_vargc( "def", x, 3 );

  if (is_cons(cadr( x )))
    require( "def",
	     is_symbol(caadr( x )),
	     "bad syntax in 'def': not a name" );

  else if (is_symbol(cadr(x)))
    require( "def",
	     fl == 3,
	     "bad syntax in 'def': too many bindings" );

  else
    error( "def", "bad syntax in 'def': not a name" );

  return cdr( x );
}

static value_t ensure_assign( value_t x ) {
  s_argc( ":=", x, 3 );
  require( ":=",
	   is_symbol(cadr(x)),
	   "bad syntax in ':=': not a name" );

  return cdr( x );
}

static value_t ensure_lambda( value_t x ) {
  s_vargc( "lmb", x, 3);

  require( "lmb",
	   is_list(cadr(x)) || is_symbol(cadr(x)),
	   "bad syntax in 'lmb': formals not a list of symbols" );

  return cdr( x );
}

static value_t ensure_formals( value_t f, size_t *n, bool *v) {
  *n = 0;
  *v = false;

  push( f );
  push( val_nil );
  
  value_t x, *f_b = &Stack[Sp-2], *r_b = &Stack[Sp-1];

  for_cons(f_b, x) {
    require( "lmb",
	     is_symbol(x),
	     "bad syntax in lmb formals" );

    *r_b = cons(x, *r_b);
    (*n)++;
  }

  if (is_symbol(*f_b)) {
    *r_b = cons(*f_b, *r_b);
    (*n)++;
    *v = true;
  } else {
    require( "lmb",
	     is_nil(*f_b),
	     "bad syntax in lmb formals" );
  }

  return popn( 2 );
}

value_t compile( value_t x ) {
  push(x);
  comp_closure( NULL, NULL, NULL, NULL, &Stack[Sp-1] );
  return pop();
}

static size_t emit_instruction( value_t *code, opcode_t op, short argx, short argy ) {
  size_t instr_len = 1;

  if ( op > op_return )
    instr_len++;

  if ( op > op_call )
    instr_len++;

  if (alength(*code) + instr_len >= asize(*code))
    *code = resize_binary(*code, alength(*code) + instr_len );

  short buf[3] = { op, argx, argy };

  memcpy( bytecode(*code), buf, instr_len * sizeof(short) );

  return alength( *code );
}

static size_t add_constant( value_t *vals, value_t x ) {
  size_t location = vector_find( *vals, x );
  
  if (location == alength(*vals)) {
    elements(*vals)[alength(*vals)++] = x;

    if (alength(*vals) == asize(*vals))
      *vals = resize_vector(*vals, asize(*vals) );
  }

  return location;
}

static opcode_t resolve_name( value_t *e, value_t *v, value_t x, short *i, short *j) {
  *i = 0;
  *j = 0;

  value_t ec = *e, l, n;
  bool found = false;

  for_cons(&ec, l) {
    for_cons(&l, n) {
      if (n == x) {
	found = true;
	break;
      }

      (*j)++;
    }

    if (found) {
      if (*i == 0) {
	*i = *j;
	return op_loadl;
      }
      return op_loadc;
    }

    *j = 0;
    (*i)++;
  }

  require( "compile",
	   is_bound(x),
	   "unbound symbol %s",
	   assymbol(x)->name );

  *i = add_constant( v, x );
  return op_loadg;
}

static size_t comp_literal( value_t *e, value_t *v, value_t *c, value_t x ) {
  (void)e;
  
  if (is_nil(x))
    return emit_instruction( c, op_loadn, 0, 0 );

  if (is_false(x))
    return emit_instruction( c, op_loadf, 0, 0 );

  if (is_true(x))
    return emit_instruction( c, op_loadt, 0, 0 );
  
  short argx = add_constant( v, x );
  
  return emit_instruction( c, op_loadv, argx, 0 );
}

static size_t comp_variable( value_t *e, value_t *v, value_t *c, value_t x ) {
  if (is_keyword(x))
    return comp_literal( e, v, c, x );

  short i, j;

  opcode_t o = resolve_name( e, v, x, &i, &j );
  return emit_instruction( c, o, i, j );
}


static size_t comp_sequence( value_t *e, value_t *v, value_t *c, value_t *s, bool acc) {
  value_t x;
  size_t n = 0;
  
  for_cons( s, x ) {
    comp_expression( e, v, c, x );

    if (!acc && !is_nil(cdr(*s)))
      emit_instruction( c, op_pop, 0, 0 );

    else
      n++;
  }

  if (acc)
    return n;

  return alength(*c);
}

static size_t comp_quote( value_t *e, value_t *v, value_t *c, value_t form) {
  value_t x = ensure_quote( form );
  return comp_literal( e, v, c, x );
}

static size_t comp_do( value_t *e, value_t *v, value_t *c, value_t form) {
  value_t x = ensure_do( form );
  push_s( "exec", x );
  
  size_t out = comp_sequence( e, v, c, &Stack[Sp-1], false );
  pop();
  return out;
}

static size_t comp_if( value_t *e, value_t *v, value_t *c, value_t form) {
  value_t x = ensure_if( form );
  size_t l1, l2;

  push( x );
  value_t *buf = Stack+Sp-1;

  x = car(*buf);
  *buf = cdr(*buf);
  
  comp_expression(e, v, c, x);
  l1 = emit_instruction( c, op_jmpf, 0, 0 ) - 1;
  
  x = car(*buf);
  *buf = cdr(*buf);
  
  comp_expression(e, v, c, x);
  l2 = emit_instruction( c, op_jmp, 0, 0 ) - 1;
  
  bytecode(*c)[l1] = alength(*c) - l1;
  
  x = is_nil(*buf) ? *buf : cdr(*buf);
  comp_expression(e, v, c, x);

  bytecode(*c)[l2] = alength(*c) - l2;

  pop();

  return alength(*c);
}

static size_t comp_define( value_t *e, value_t *v, value_t *c, value_t form ) {
  value_t x = ensure_define( form );
  push_s( "comp", x );
  value_t *buf = &Stack[Sp-1];

  bool is_defun = is_cons(car(*buf));

  value_t name = is_defun ? caar(*buf) : car(*buf);

  opcode_t op; short argx, argy = 0;

  if (is_nil(*e)) { // toplevel definition
    if (!is_bound(name))
      assymbol(name)->bind = val_nil;

    op   = op_storeg;
    argx = add_constant( v, name );
  } else {
    emit_instruction( c, op_loadn, 0, 0 ); // ensures the space
    op = op_storel;
    value_t l = car(*e);
    argx = list_length( l );
    l = cons( name, l );
    car(*e) = l;
  }

  if (is_defun) {
    push_s( "comp", car(*buf) );
    Stack[Sp-1] = cdr(Stack[Sp-1]);
    Stack[Sp-2] = cdr(Stack[Sp-2]);
    comp_closure( e, v, c, &Stack[Sp-1], &Stack[Sp-2]);
    popn(2);
  } else {
    comp_expression( e, v, c, cadr(*buf) );
  }

  pop();
  
  return emit_instruction( c, op, argx, argy );
}

static size_t comp_assign( value_t *e, value_t *v, value_t *c, value_t form ) {
  value_t x = ensure_assign( form );
  push_s( "comp", x );
  value_t *buf = &Stack[Sp-1];
  opcode_t op; short argx, argy;

  op = resolve_name(e, v, car(*buf), &argx, &argy );

  comp_expression( e, v, c, cadr(*buf) );

  pop();

  return emit_instruction( c, op, argx, argy );
}

static size_t comp_lambda( value_t *e, value_t *v, value_t *c, value_t form ) {
  value_t x = ensure_lambda( form );
  
  push_s( "comp", car(x));
  push_s( "comp", cdr(x));

  size_t out = comp_closure( e, v, c, &Stack[Sp-2], &Stack[Sp-1]);
  popn(2);
  return out;
}


static size_t comp_closure( value_t *e, value_t *v, value_t *c, value_t *f, value_t *s ) {
  
  index_t envt_buffer = push( val_nil );                    // toplevel environment
  index_t vals_buffer = vector_s( 8, NULL );                // constants
  index_t code_buffer = binary_s( 32, C_sint16, NULL);      // instructions

  bool is_toplevel = false;
  

  if ((is_toplevel=(e==NULL)))
    e = &Stack[envt_buffer];

  if (v==NULL)
    v = &Stack[vals_buffer];

  if (c==NULL)
    c = &Stack[code_buffer];

  if (f==NULL)
    f = s;

  if (is_toplevel) {
    
    comp_expression( e, v, c, *s );
    emit_instruction( c, op_return, 0, 0 );

    // create the toplevel closure now
    closure_s( e, v, c );

  } else {
    size_t n_formals; bool vargs;

    value_t formals = ensure_formals( *f, &n_formals, &vargs );

    Stack[envt_buffer] = *e;
    Stack[envt_buffer] = cons( formals, Stack[envt_buffer] );

    emit_instruction( &Stack[code_buffer], (vargs ? op_argc : op_vargc), n_formals, 0 );
    
    comp_sequence( &Stack[envt_buffer], &Stack[vals_buffer], &Stack[code_buffer], s, false );

    // save code and constants in calling closure 
    size_t vloc = vector_put( *v, Stack[vals_buffer] );
    size_t cloc = vector_put( *v, Stack[code_buffer] );
    size_t ncap = cons_length( car(*e) );

    // arrange for a closure to be created at runtime
    emit_instruction( c, op_capture, ncap, 0 );
    emit_instruction( c, op_loadv, vloc, 0 );
    emit_instruction( c, op_loadv, cloc, 0 );
    emit_instruction( c, op_closure, 0, 0 );
  }

  return alength(*c);
}


static bool is_quote( value_t form ) { return car( form ) == r_quote; }
static bool is_do( value_t form ) { return car( form ) == r_do; }
static bool is_if( value_t form ) { return car( form ) == r_if; }
static bool is_define( value_t form ) { return car( form ) == r_define; }
static bool is_assign( value_t form ) { return car( form ) == r_assign; }
static bool is_lambda( value_t form ) { return car( form ) == r_lambda; }

static size_t comp_expression( value_t *e, value_t *v, value_t *c, value_t x ) {  
  if (is_symbol(x))
    return comp_variable( e, v, c, x );

  else if (!is_cons(x))
    return comp_literal( e, v, c, x );

  else if (is_quote(x))
    return comp_quote( e, v, c, x );

  else if (is_do(x))
    return comp_do(e, v, c, x);

  else if (is_if(x))
    return comp_if(e, v, c, x);

  else if (is_define(x))
    return comp_define(e, v, c, x);

  else if (is_assign(x))
    return comp_assign(e, v, c, x);

  else if (is_lambda(x))
    return comp_lambda(e, v, c, x);

  // compile procedure call
  index_t args_buf = push_s( "comp", cdr( x ) );
  comp_expression( e, v, c, car(x) );
  size_t nargs = comp_sequence( e, v, c, &Stack[args_buf], true );
  pop();
  return emit_instruction( c, op_call, nargs, 0 );
}

#define Func Stack[Bp-1]
#define Narg Stack[Fp-1]
#define Next Stack[Fp]
#define RPC  Stack[Fp+1]
#define Code Stack[Fp+2]
#define Vals Stack[Fp+3]
#define Envt Stack[Fp+4]
#define Args (Stack+Bp)
#define Vars (Stack+Fp+4)
#define Valu Stack[Sp-1]

static bool is_captured( index_t fp ) {
  return ival( Stack[fp-1] ) == -1;
}

value_t execute( value_t code ) {
  push_s( "exec", code );
  return apply( 0 );
}

value_t apply( size_t nargs ) {
  static void *labels[num_opcodes] = {
    /* 0-argument instructions */
    [op_return] = &&do_return,

    [op_pop]    = &&do_pop,

    [op_loadn]  = &&do_loadn, [op_loadt] = &&do_loadt, [op_loadf] = &&do_loadf,

    [op_closure] = &&do_closure,
    /* 1-argument instructions */
    [op_loadv]  = &&do_loadv,

    [op_loadl]  = &&do_loadl, [op_storel] = &&do_storel,

    [op_loadg]  = &&do_loadg, [op_storeg] = &&do_storeg,

    [op_jmp] = &&do_jmp, [op_jmpf] = &&do_jmpf, [op_jmpt] = &&do_jmpt,

    [op_argc]   = &&do_argc,  [op_vargc]  = &&do_vargc,
    
    [op_capture] = &&do_capture,  [op_call]   = &&do_call,

    /* 2-argument instructions */
    [op_loadc] = &&do_loadc, [op_storec] = &&do_storec
  };

  
  opcode_t op;
  short argx = nargs, // allows 
    argy = 0;
  value_t x, y, z, f;

 do_call:
  f = Stack[Sp-argx-1];

  require( "apply",
	   is_function(f),
	   "# wanted a function()" );

  if (is_builtin(f)) { // the builtin manages the stack itself
    asbuiltin(f)->callback( argx );
    
  } else { // set up new stack frame
    x = fixnum(argx);
    y = fixnum(Fp);
    z = fixnum(Pc);
    
    Pc = 0;
    Bp = Fp - argx;

    push(x);
    
    Fp = push(y);
    
    push(z);

    push(asclosure(f)->code);
    push(asclosure(f)->vals);
    push(asclosure(f)->envt);
  }

 do_fetch:
  op = bytecode(Code)[Pc++];

  if (op > op_loadf)
    argx = bytecode(Code)[Pc++];

  if (op > op_call)
    argy = bytecode(Code)[Pc++];

  goto *labels[op];

 do_pop:
  pop_s( "exec" );
  
  goto do_fetch;

 do_loadn:
  push_s( "exec", val_nil );

  goto do_fetch;

 do_loadt:
  push_s( "exec", val_true );
  
  goto do_fetch;

 do_loadf:
  push_s( "exec", val_false );
  
  goto do_fetch;

 do_closure:
  closure_s( &Stack[Sp-3], &Stack[Sp-2], &Stack[Sp-1] );

  goto do_fetch;

 do_loadv:
  x = elements(Vals)[argx];
  push_s( "exec", x );

  goto do_fetch;

 do_loadl:
  argy = ival( Narg );
  
  if (is_captured(Fp))
    x = elements(Envt)[argx+1];

  else if (argx > argy )
    x = Vars[argx-argy];

  else
    x = Args[argx];

  push_s( "exec", x );

  goto do_fetch;

 do_storel:
  x = Stack[Sp-1];
  argy = ival( Narg );
  
  if (is_captured(Fp))
    elements(Envt)[argx+1] = x;

  else if (argx > argy)
    Vars[argx-argy] = x;

  else
    Args[argx] = x;

  goto do_fetch;

 do_loadg:
  y = elements(Vals)[argx];
  x = assymbol(y)->bind;
  push_s( "exec", x );

  goto do_fetch;

 do_jmp:
  Pc += argx;
  
  goto do_fetch;

 do_jmpf:
  x = pop();

  if (!Cbool(x))
    Pc += argx;

  goto do_fetch;

 do_jmpt:
  x = pop();

  if (Cbool(x))
    Pc += argx;

  goto do_fetch;

 do_storeg:
  y = elements(Vals)[argx];
  x = Stack[Sp-1];
  assymbol(y)->bind = x;

  goto do_fetch;

 do_argc:
  argy = ival( Narg );
  argc( "exec", argy, argx );

  goto do_fetch;

 do_vargc:
  argy = ival( Narg );

  vargc( "exec", argy, argx );

  // frame needs to be adjusted 
  x = Next;
  y = RPC;

  popn( 6 );
  list_s( argy - argx, Args+argx );
  push( fixnum( argx ) );
  Fp = push( x );
  push( y );
  push( asclosure( Func )->code );
  push( asclosure( Func )->vals );
  push( asclosure( Func )->envt );

  goto do_fetch;
  
 do_capture:
  if (!is_captured(Fp)) {
    argy = ival( Narg );
    
    vector_s( argx+1, NULL );
    elements( Valu )[0] = Envt;
    memcpy( &elements( Valu )[1], Args, argy * sizeof(value_t) );
    memcpy( &elements( Valu )[1+argx-argy], Vars, (argx-argy) * sizeof(value_t));
    Narg = fixnum( -1 );
    Envt = pop();
  }

  push_s( "exec", Envt );

  goto do_fetch;

 do_loadc:
  y = Envt;

  while (argx-- > 0)
    y = elements(y)[0];

  x = elements(y)[argy];

  push_s( "exec", x );

  goto do_fetch;

 do_storec:
  y = Envt;

  while (argx-- > 0)
    y = elements(y)[0];

  elements(y)[argy] = Stack[Sp-1];

  goto do_fetch;
  
 do_return:
  x  = Stack[Sp-1];

  Sp   = Fp+3;
  Pc   = ival( pop() );
  Fp   = ival( pop() );
  argx = ival( pop() );
  Bp   = Fp - ival( Stack[Fp-1] );

  popn( argx+1 );

  if (Fp == 0) // execution done
    return x;

  goto do_fetch;
}

void r_builtin(comp) {
  argc( "comp", n, 1 );
  value_t res = compile( Stack[Sp-1] );
  Stack[Sp-1] = res;
}

void r_builtin(exec) {
  argc( "exec", n, 1 );
  argt( "exec", Stack[Sp-1], type_closure );
  value_t res = execute( pop() );
  push( res );
}

void r_builtin(apply) {
  vargc( "apply", n, 2 );
  require( "apply",
	   is_function(Sref(2)),
	   "# wanted a function()" );

  size_t n_apply = n-1;
  n = 0;
  value_t buf[n_apply];

  memcpy( buf, &Sref(n_apply), (n_apply) * sizeof(value_t) );
  popn(n_apply);

  for (size_t i=0; i<n-1; i++) {
    if (is_empty(buf[i]))
      continue;
    
    if (is_list(buf[i]))
      unpack_cons(&buf[i], n);

    else if (is_vector(buf[i]))
      unpack_vector(buf[i], n);

    else if (is_binary(buf[i]))
      unpack_binary(buf[i], n);

    else if (is_string(buf[i]))
      unpack_string(buf[i], n);

    else
      unpack_atom(buf[i], n);
  }

  value_t x = apply( n );
  push( x );
}

// initialization -------------------------------------------------------------
void init_vm( void ) {

  // special forms ------------------------------------------------------------
  r_quote  = symbol("quote");
  r_lambda = symbol("lmb");
  r_define = symbol("def");
  r_assign = symbol(":=");
  r_do     = symbol("do");
  r_if     = symbol("if");

  // builtin functions --------------------------------------------------------
  mk_builtin(apply);
  mk_builtin(comp);
  mk_builtin(exec);
}
