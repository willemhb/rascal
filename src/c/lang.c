#include <string.h>
#include <stdarg.h>

#include "lang.h"
#include "object.h"
#include "runtime.h"

#include "opcodes.h"

#include "util/hashing.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t Quote, Do, If, Lmb, Def, Put, Amp, Otherwise;

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return *as_symbol(x)->name == ':';

  if ( is_list(x) )
    return as_list(x)->arity > 0;

  return true;
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t read( value_t src );
value_t print( FILE* ios, value_t x );

value_t eval( value_t x ) {
  if ( is_literal(x) )
    return x;

  if ( is_symbol(x) ) {
    value_t out = toplevel_lookup(x);
    forbid("eval", out == NOTFOUND, x, "unbound symbol");
    return out;
  }

  value_t chunk = compile(x);
  value_t out = exec(chunk);
  return out;
}

static usize emit_instr( chunk_t* target, opcode_t op, ... );
static usize add_value( chunk_t* target, value_t val );
static bool is_toplevel_form( chunk_t* target );
static opcode_t resolve_name( chunk_t* target, value_t name, usize* i, usize* j );
static usize define_global( value_t name );
static usize define_local( chunk_t* target, value_t name );
static usize compile_value( chunk_t* target, value_t val );
static usize compile_variable( chunk_t* target, value_t name );
static usize compile_combination( chunk_t* target, value_t sexpr );
static usize compile_expr( chunk_t* target, value_t src );
static usize compile_exprs( chunk_t* target, list_t* exprs );
static usize compile_args( chunk_t* target, list_t* args );
static usize compile_quote( chunk_t* target, list_t* form );
static usize compile_do( chunk_t* target, list_t* form );
static usize compile_if( chunk_t* target, list_t* form );
static usize compile_lmb( chunk_t* target, list_t* form );
static usize compile_def( chunk_t* target, list_t* form );
static usize compile_put( chunk_t* target, list_t* form );

value_t compile( value_t src ) {
  chunk_t* target = chunk( &EmptyList );
  compile_expr(target, src);
  return object(target);
}

static usize emit_instr( chunk_t* target, opcode_t op, ... ) {
  va_list va;
  va_start(va, op);

  ushort buf[3] = { op, 0, 0 };
  usize n = opcode_argc(op);

  if ( n > 0 )
    buf[1] = va_arg(va, int);

  if ( n > 1 )
    buf[2] = va_arg(va, int);

  va_end(va);
  buffer_write(&target->instr, n+1, buf);
  return target->instr.cnt;
}

static usize add_value( chunk_t* target, value_t val ) {
  return values_push(&target->vals, val);
}

static bool is_toplevel_form( chunk_t* target ) {
  return target->envt->arity == 0; // target is for a toplevel from if its environment is the empty list
}

static opcode_t resolve_name( chunk_t* target, value_t name, usize* i, usize* j ) {
  opcode_t out = OP_LOADCL;
  *i = *j = 0;

  list_t* envt = target->envt;

  while ( envt->arity ) {
    list_t* locals = as_list(envt->head);

    while ( locals->arity ) {
      if ( locals->head == name ) {
        *j = locals->arity;
        break;
      }

      locals = locals->tail;
    }

    if ( *j )
      break;

    *i = *i + 1;
    envt = envt->tail;
  }

  if ( *j == 0 ) { // not found
    *i = define_global(name);
    out = OP_LOADGL;
  }

  return out;
}

static usize define_global( value_t name ) {
  value_t location = table_add(&Vm.globals.vars, name, number(Vm.globals.vars.cnt));

  if ( location == NOTFOUND )
    location = number(values_push(&Vm.globals.vals, UNDEFINED));

  return wrdval(location);
}

static usize define_local( chunk_t* target, value_t name ) {
  list_t* envt = target->envt;
  list_t* locals = as_list(envt->head);
  set_head(envt, object(list(name, locals)));
  return locals->arity + 1;
}

static usize compile_value( chunk_t* target, value_t val ) {
  usize location = add_value(target, val);
  return emit_instr(target, OP_LOADV, location);
}

static usize compile_variable( chunk_t* target, value_t name ) {
  usize i, j;
  opcode_t op = resolve_name(target, name, &i, &j);
  return emit_instr(target, op, i, j);
}

static usize compile_combination( chunk_t* target, value_t sexpr ) {
  list_t* form = as_list(sexpr);

  if ( form->head == Quote )
    return compile_quote(target, form);

  if ( form->head == Do )
    return compile_do(target, form);

  if ( form->head == If )
    return compile_if(target, form);

  if ( form->head == Lmb )
    return compile_lmb(target, form);

  if ( form->head == Def )
    return compile_def(target, form);

  if ( form->head == Put )
    return compile_put(target, form);

  compile_expr(target, form->head); // compile caller
  compile_args(target, form->tail); // compile arguments
  return emit_instr(target, OP_CALL, form->tail->arity);
}

static usize compile_expr( chunk_t* target, value_t expr ) {
  usize out;
  
  if ( is_literal(expr) )
    out = compile_value(target, expr);

  else if ( is_symbol(expr) )
    out = compile_variable(target, expr);

  else
    out = compile_combination(target, expr);

  return out;
}

static usize compile_exprs( chunk_t* target, list_t* exprs ) {
  while ( exprs->arity > 1 ) {
    compile_expr(target, exprs->head);
    emit_instr(target, OP_POP);
    exprs = exprs->tail;
  }

  return compile_expr(target, exprs->head);
}

static usize compile_args( chunk_t* target, list_t* args ) {
  usize out = target->instr.cnt;
  
  while ( args->arity ) {
    out = compile_expr(target, args->head);
    args = args->tail;
  }

  return out;
}

value_t apply( value_t f, value_t a );

value_t exec( value_t ch ) {
  static void* labels[] = {
    [OP_NOOP] = &&op_noop, [OP_START] = &&op_start, [OP_POP] = &&op_pop,
    [OP_CLOSURE] = &&op_closure, [OP_RETURN] = &&op_return,

    [OP_ARGC] = &&op_argc, [OP_VARGC] = &&op_vargc,
    [OP_CALL] = &&op_call,
    [OP_LOADV] = &&op_loadv, [OP_LOADGL] = &&op_loadgl,
    [OP_PUTGL] = &&op_putgl, [OP_JUMP] = &&op_jump, [OP_JUMPN] = &&op_jumpn,

    [OP_LOADCL] = &&op_loadcl, [OP_PUTCL] = &&op_putcl
  };

  opcode_t op;
  int argx, argy;
  value_t x, v, * b;
  list_t* va;

 fetch: // next instruction
  op = *(IP++);

  if ( op > OP_RETURN )
    argx = *(IP++);

  if ( op > OP_JUMPN )
    argy = *(IP++);

  goto *labels[op];

 op_start:
  goto fetch;
  
 op_noop:
  goto fetch;

 op_pop:
  pop();
  goto fetch;

 op_closure:
  v = pop();
  capture_frame(FP);
  v = object(closure(as_chunk(v), ENV));
  push(v);

  goto fetch;

 op_return:
  v = pop();
  pop_frame();

  if ( FN == NULL )
    return v;

  push(v);

  goto fetch;

 op_argc:
  argy = SP - BP - 1;
  require("exec", argy == argx, object(FN), "Incorrect arity");

  goto pad_locals;

 op_vargc:
  argy = SP - BP - 1;
  require("exec", argy >= argx, object(FN), "Incorrect arity");
  va = mk_list(argy - argx, BP+argx+1);
  BP[argx+1] = object(va);
  SP = BP+argx+2;

  goto pad_locals;

 pad_locals: // adjust stack to hold all local bindings, including those introduced in function body
  for ( int i=argx; i<(int)FN->envt->arity; i++ )
    push(NIL);

  goto fetch;

 op_call:
  x = SP[-argx-1];

  if ( is_native(x) )
    goto do_call_native;

  else if ( is_closure(x) )
    goto do_call_closure;

  else
    error("exec", x, "not a function");

 do_call_native:
  v = as_native(x)(argx, Vm.sp-argx);
  Vm.sp[-argx-1] = v;
  Vm.sp -= argx;

  goto fetch;

 do_call_closure:
  push_frame();

  FN = as_closure(x)->code;
  BP = SP-argx-1;
  IP = FN->instr.data;
  BP[0] = object(as_closure(x)->envt);

  goto fetch;
  
 op_loadv:
  push(FN->vals.data[argx]);
  goto fetch;
  
 op_loadgl:
  push(Vm.globals.vals.data[argx]);
  goto fetch;

 op_putgl:
  Vm.globals.vals.data[argx] = Vm.sp[-1];
  goto fetch;
  
 op_jump:
  IP += argx;
  goto fetch;

 op_jumpn:
  x = pop();

  if ( x == NIL )
    IP += argx;

  goto fetch;

 op_loadcl:
  b = ENV ? ENV->slots : BP;
  while ( argx-- )
    b = as_tuple(b[0])->slots;
  push(b[argy+1]);
  goto fetch;

 op_putcl:
  b = ENV ? ENV->slots : BP;
  while ( argx-- )
    b = as_tuple(b[0])->slots;
  b[argy+1] = Vm.sp[-1];
  goto fetch;
}

uhash hash( value_t x ) {
  uhash out;
  
  if ( is_object(x) ) {
    if ( !hasfl(x, HASHED) ) {
      datatype_t dt = type_of(x);
      uhash nh = hash_uword(dt);

      switch ( type_of(x) ) {
        case SYMBOL: {
          symbol_t* s = as_symbol(x);
          nh = mix_3_hashes(nh, hash_str(s->name), hash_uword(s->idno));
          break;
        }

        case LIST: {
          list_t* l = as_list(x);

          if ( l == &EmptyList )
            nh  = mix_2_hashes(nh, nh);

          else
            for ( ; l->arity; l=l->tail )
              nh = mix_2_hashes(nh, hash(l->head));

          break;
        }

        case TUPLE: {
          tuple_t* t = as_tuple(x);

          if ( t == &EmptyTuple )
            nh = mix_2_hashes(nh, nh);

          else
            for ( usize i=0; i<t->arity; i++ )
              nh = mix_2_hashes(nh, hash(t->slots[i]));

          break;
        }

        case CHUNK: {
          chunk_t* c = as_chunk(x);
          nh = mix_n_hashes(4, nh, hash(object(c->envt)), hash_buffer(&c->instr), hash_values(&c->vals));
          break;
        }

        case CLOSURE: {
          closure_t* cl = as_closure(x);
          nh = mix_3_hashes(nh, hash(object(cl->code)), hash(object(cl->envt)));
          break;
        }

        default:
          unreachable();
      }

      sethash(x, nh);
    }
    
    out = as_object(x)->hash;
  } else {
    out = hash_uword(x);
  }

  return out;
}

int compare( value_t x, value_t y, bool eq ) {
  int out = 0;

  if ( x == y )
    out = 0;
  
  else if ( eq ) {
    datatype_t tx = type_of(x), ty = type_of(y);

    if ( tx != ty )
      out = 1;

    else {
      switch ( tx ) {
        case LIST: {
          list_t* lx = as_list(x), * ly = as_list(y);

          if ( lx->arity != ly->arity )
            out = 1;

          else
            for ( ; lx->arity && out == 0; lx=lx->tail, ly=ly->tail )
              out = compare(lx->head, ly->head, true);

          break;
        }


        case TUPLE: {
          tuple_t* tx = as_tuple(x), * ty = as_tuple(y);

          if ( tx->arity != ty->arity )
            out = 1;

          else
            for ( usize i=0; i<tx->arity && out == 0; i++ )
              out = compare(tx->slots[i], ty->slots[i], true);

          break;
        }

        case CLOSURE: {
          closure_t* cx = as_closure(x), * cy = as_closure(y);
          out = compare(object(cx->code), object(cy->code), true);
          out = out && compare(object(cx->envt), object(cy->envt), true);
          break;
        }

        case CHUNK: {
          chunk_t* cx = as_chunk(x), * cy = as_chunk(y);
          out = compare(object(cx->envt), object(cy->envt), true);
          out = out && compare_buffers(&cx->instr, &cy->instr, true);
          out = out && compare_values(&cx->vals, &cy->vals, true);
          break;
        }
          
        default:
          out = 1;
          break;
      }
    }
  } else {
    datatype_t tx = type_of(x), ty = type_of(y);

    if ( tx != ty )
      out = CMP(tx, ty);

    else {
      switch ( tx ) {
        case NUMBER: {
          number_t nx = as_number(x), ny = as_number(y);
          out = CMP(nx, ny);
          break;
        }

        case SYMBOL: {
          symbol_t* sx = as_symbol(x), * sy = as_symbol(y);
          
          out = strcmp(sx->name, sy->name);

          if ( out == 0 )
            out = CMP(sx->idno, sy->idno);

          break;
        }

        case LIST: {
          list_t* lx = as_list(x), * ly = as_list(y);

          for ( ; lx->arity && ly->arity && out == 0; lx=lx->tail, ly=ly->tail )
            out = compare(lx->head, ly->head, false);

          if ( out == 0 )
            out = 0 - (lx->arity < ly->arity) + (lx->arity > ly->arity);

          break;
        }

        case TUPLE: {
          tuple_t* tx = as_tuple(x), * ty = as_tuple(y);

          for ( usize i=0; i<tx->arity && i<ty->arity && out == 0; i++ )
            out = compare(tx->slots[i], ty->slots[i], false);

          if ( out == 0 )
            out = 0 - (tx->arity < ty->arity) + (tx->arity > ty->arity);

          break;
        }

        case CLOSURE: {
          closure_t* cx = as_closure(x), * cy = as_closure(y);

          out = compare(object(cx->code), object(cy->code), false);
          out = out ? : compare(object(cx->envt), object(cy->envt), false);

          break;
        }

        case CHUNK: {
          chunk_t* cx = as_chunk(x), * cy = as_chunk(y);

          out = compare(object(cx->envt), object(cy->envt), false);
          out = out ? : compare_buffers(&cx->instr, &cy->instr, false);
          out = out ? : compare_values(&cx->vals, &cy->vals, false);

          break;
        }

        default:
          out = CMP(x, y);
          break;
      }
    }
  }

  return out;
}

// initialization -------------------------------------------------------------
void toplevel_init_lang( void );
