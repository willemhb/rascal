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

uhash hash( value_t x ) {
  uhash out;
  
  if ( is_object(x) ) {
    if ( !hasfl(x, HASHED) ) {
      uhash nh;

      if ( !hasfl(x, FROZEN) ) // mutable objects always hash on identity
        nh = hash_uword(x);

      else {
        datatype_t dt = type_of(x);
        nh = hash_uword(dt);

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
            nh = hash_uword(x);
            break;
        }
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
