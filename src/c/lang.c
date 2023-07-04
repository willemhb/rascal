#include <string.h>

#include "lang.h"
#include "object.h"
#include "runtime.h"

#include "opcodes.h"

#include "util/hashing.h"

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return *as_symbol(x)->name == ':';

  if ( is_list(x) )
    return as_list(x)->arity > 0;

  return true;
}

static void init_envt( void ) {
  if ( !!(Vm.fn->envt->obj.flags & VARIADIC) ) {
    
  }
}

static void capture_values( void ) {
  value_t* bp = Vm.bp;

  while ( in_stack(bp) ) {
    
  }
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

value_t compile( value_t src ) {}

value_t apply( value_t f, value_t a );

value_t exec( value_t ch ) {
  static void* labels[] = {
    [OP_NOOP] = &&op_noop, [OP_START] = &&op_start, [OP_POP] = &&op_pop,
    [OP_CLOSURE] = &&op_closure, [OP_RETURN] = &&op_return,

    [OP_CALL] = &&op_call,
    [OP_LOADV] = &&op_loadv, [OP_LOADGL] = &&op_loadgl,
    [OP_PUTGL] = &&op_putgl, [OP_JUMP] = &&op_jump, [OP_JUMPN] = &&op_jumpn,

    [OP_LOADCL] = &&op_loadcl, [OP_PUTCL] = &&op_putcl
  };

  opcode_t op;
  int argx, argy;
  value_t x, v, * b;

 fetch:
  op = *(Vm.ip++);

  if ( op > OP_RETURN )
    argx = *(Vm.ip++);

  if ( op > OP_JUMPN )
    argy = *(Vm.ip++);

  goto *labels[op];

 op_start:
  goto fetch;
  
 op_noop:
  goto fetch;

 op_pop:
  pop();
  goto fetch;

 op_closure:
  
  
 op_return:
  v = pop();
  Vm.sp = Vm.bp;
  Vm.bp = (value_t*)wrdval(restore());
  Vm.ip = (ushort*)wrdval(restore());
  Vm.fn = (chunk_t*)as_object(restore());

  if ( Vm.fn == NULL )
    return v;

  push(v);

  goto fetch;

 op_call:
  x = Vm.sp[-argx-1];

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
  save(object(Vm.fn));
  save(pointer(Vm.ip));
  save(pointer(Vm.bp));

  Vm.fn = as_closure(x)->code;
  Vm.bp = Vm.sp-argx-1;
  Vm.ip = Vm.fn->instr.data;
  Vm.bp[0] = object(as_closure(x)->envt);
  init_envt();

  goto fetch;
  
 op_loadv:
  push(Vm.fn->vals.data[argx]);
  goto fetch;
  
 op_loadgl:
  push(Vm.globals.vals.data[argx]);
  goto fetch;

 op_putgl:
  Vm.globals.vals.data[argx] = Vm.sp[-1];
  goto fetch;
  
 op_jump:
  Vm.ip += argx;
  goto fetch;

 op_jumpn:
  x = pop();

  if ( x == NIL )
    Vm.ip += argx;

  goto fetch;

 op_loadcl:
  b = Vm.bp;
  while ( argx-- )
    b = as_tuple(b[0])->slots;
  push(b[argy]);
  goto fetch;
  
 op_putcl:
  b = Vm.bp;
  while ( argx-- )
    b = as_tuple(b[0])->slots;
  b[argy] = Vm.sp[-1];
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
          nh = mix_3_hashes(nh, hash_buffer(&c->instr), hash_values(&c->vals));
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
          out = compare_buffers(&cx->instr, &cy->instr, true);
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

          out = compare_buffers(&cx->instr, &cy->instr, false);
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
