#include <string.h>
#include <stdarg.h>

#include "lang.h"
#include "object.h"
#include "runtime.h"

#include "opcodes.h"

#include "util/hashing.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// read implementation --------------------------------------------------------
value_t read( port_t src );

// print implementation -------------------------------------------------------
value_t print( port_t ios, value_t x );

// hash implementation --------------------------------------------------------
static uhash hash_symbol( symbol_t* s ) {
  assert(s);
  uhash type_hash = hash_uword(SYMBOL);
  uhash name_hash = hash_str(s->name);
  uhash idno_hash = hash_uword(s->idno);
  return mix_3_hashes(type_hash, name_hash, idno_hash);
}

static uhash hash_list( list_t* l ) {
  assert(l);
  uhash type_hash = hash_uword(LIST);
  uhash output_hash = type_hash;

  while ( l->arity ) {
    uhash head_hash = hash(l->head);
    output_hash = mix_2_hashes(output_hash, head_hash);
    l = l->tail;
  }

  return output_hash;
}

uhash hash( value_t x ) {
  uhash out;
  
  if ( is_object(x) ) {
    if ( !hasfl(x, HASHED) ) {
      uhash nh;

      if ( !hasfl(x, FROZEN) ) // mutable objects always hash on identity
        nh = hash_uword(x);

      else {
        switch ( type_of(x) ) {
          case SYMBOL: nh = hash_symbol(as_symbol(x)); break;
          case LIST:   nh = hash_list(as_list(x));     break;
          default:     nh = hash_uword(x);             break;
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

// same implementation --------------------------------------------------------
bool same( value_t x, value_t y ) {
  return x == y;
}

// equal implementation 

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
