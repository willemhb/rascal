#include "compare.h"
#include "object.h"

#include "util/hashing.h"

// hash +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uhash hash_object( void* ptr, bool id ) {
  return hash_value(object(ptr), id);
}

uhash hash_value( value_t x, bool id ) {
  uhash out;
  
  if ( id )
    out = hash_uword(x);

  else
    switch ( type_of(x) ) {
      case SYMBOL:
        out = as_symbol(x)->hash;
        break;

      case LIST: {
        list_t* xs = as_list(x);
        out = hash_uword(LIST);

        for ( ; xs->arity; xs=xs->tail )
          out = mix_2_hashes(out, hash_value(xs->head, false));

        break;
      }

      case BINARY: {
        binary_t* bs = as_binary(x);
        out = mix_2_hashes(hash_uword(BINARY), hash_mem(bs->data, bs->cnt * bs->elSize));
        break;
      }

      case VECTOR: {
        vector_t* vs = as_vector(x);
        out = hash_uword(VECTOR);

        for ( usize i=0; i < vs->cnt; i++ )
          out = mix_2_hashes(out, hash_value(vs->data[i], false));

        break;
      }

      case TABLE: {
        table_t* ks = as_table(x);
        out = hash_uword(TABLE);

        for ( usize i=0; i < ks->cnt; i++ ) {
          value_t k = ks->data[i*2], v = ks->data[i*2+1];
          
          out = mix_3_hashes(out, hash_value(k, false), hash_value(v, false));
        }

        break;
      }

      case NATIVE: {
        native_t* fx = as_native(x);
        out = mix_2_hashes(hash_uword(NATIVE), fx->name->hash);
        break;
      }

      case CLOSURE: {
        closure_t* fx = as_closure(x);
        out = hash_uword(CLOSURE);
        uhash h1 = hash_object(fx->code, false);
        uhash h2 = hash_object(fx->envt, false);
        out = mix_3_hashes(out, h1, h2);
        break;
      }

      case CHUNK: {
        chunk_t* cx = as_chunk(x);
        out = hash_uword(CHUNK);
        uhash h1 = cx->name->hash;
        uhash h2 = hash_object(cx->envt, false);
        uhash h3 = hash_object(cx->vals, false);
        uhash h4 = hash_object(cx->code, false);
        out = mix_n_hashes(5, out, h1, h2, h3, h4);
        break;
      }

      default:
        out = hash_uword(x);
        break;
    }

  return out;
}
// compare --------------------------------------------------------------------
int compare_values( value_t x, value_t y, bool id, bool eq ) {
  int out = 0;
  
  if ( eq ) {
    out = x != y;
    
    if ( out && !id ) {
      out = 0;
      type_t xt = type_of(x), yt = type_of(y);

      if ( xt != yt )
        out = 1;

      else {
        switch ( xt ) {
          case LIST:{
            list_t* lsx = as_list(x), * lsy = as_list(y);
            
            if ( lsx->arity != lsy->arity )
              out = 1;

            else
              for (out=0; out == 0 && lsx->arity > 0; lsx=lsx->tail, lsy=lsy->tail )
                out = compare_values(lsx->head, lsy->head, false, true);

            break;
          }

          case BINARY: {
            binary_t* bx = as_binary(x), * by = as_binary(y);

            if ( bx->elSize != by->elSize )
              out = 1;

            else if ( bx->encoded != by->encoded )
              out = 1;

            else if ( bx->cnt != by->cnt )
              out = 1;

            else
              out = !!memcmp(bx->data, by->data, bx->cnt * bx->elSize);

            break;
          }

          case VECTOR: {
            vector_t* vx = as_vector(x), * vy = as_vector(y);

            if ( vx->cnt != vy->cnt )
              out = 1;

            else
              for ( usize i=0; out == 0 && i < vx->cnt; i++ )
                out = compare_values(vx->data[i], vy->data[i], false, true);

            break;
          }

          case TABLE: {
            table_t* kvx = as_table(x), * kvy = as_table(y);

            if ( kvx->cnt != kvy->cnt )
              out = 1;

            else
              for ( usize i=0; out == 0 && i < kvx->cnt; i++ ) {
                value_t k1 = kvx->data[i*2], v1 = kvx->data[i*2+1];
                value_t k2 = kvy->data[i*2], v2 = kvy->data[i*2+1];

                out = compare_values(k1, k2, false, true);

                if ( out == 0 )
                  out = compare_values(v1, v2, false, true);
              }

            break;
          }

          case CLOSURE: {
            closure_t* clx = as_closure(x), * cly = as_closure(y);

            out = compare_objects(clx->code, cly->code, false, true);

            if ( out == 0 )
              out = compare_objects(clx->envt, cly->envt, false, true);

            break;
          }

          case CHUNK: {
            chunk_t* chx = as_chunk(x), * chy = as_chunk(y);

            out = chx->name != chy->name;
            out = out || compare_objects(chx->envt, chy->envt, false, true);
            out = out || compare_objects(chx->vals, chy->vals, false, true);
            out = out || compare_objects(chx->code, chy->code, false, true);

            break;
          }

          default:
            out = 1;
            break;
        }
      }
    }
  } else {
    if ( x != y ) { 
      type_t xt = type_of(x), yt = type_of(y);

      if ( xt != yt )
        out = 0 - (xt < yt) + (xt > yt);

      else {
        switch ( xt ) {
          case SYMBOL: {
            symbol_t* sx = as_symbol(x), * sy = as_symbol(y);

            out = strcmp(sx->name, sy->name);

            if ( out == 0 )
              out = CMP(sx->idno, sy->idno);

            else // normalize
              out = 0 - (out < 0) + (out > 0);

            break;
          }

          case LIST: {
            list_t* lx = as_list(x), * ly = as_list(y);

            for ( ; out == 0 && lx->arity && ly->arity; lx=lx->tail, ly=ly->tail )
              out = compare_values(lx->head, ly->head, false, false);

            if ( out == 0 )
              out = 0 - !!lx->arity + !!ly->arity;

            break;
          }

          case BINARY: {
            binary_t* bx = as_binary(x), * by = as_binary(y);

            if ( bx->elSize != by->elSize )
              out = 0 - (bx->elSize < by->elSize) + (bx->elSize > by->elSize);

            else if ( bx->encoded != by->encoded )
              out = 0 - bx->encoded + by->encoded;

            else {
              usize maxc = MAX(bx->cnt*bx->elSize, by->cnt*by->elSize);
              out = memcmp(bx->data, by->data, maxc);
              out = 0 - (out < 0) + (out > 0); // normalize
            }

            break;
          }

          case VECTOR: {
            vector_t* vx = as_vector(x), * vy = as_vector(y);
            usize maxc = MAX(vx->cnt, vy->cnt);

            for ( usize i=0; out == 0 && i < maxc; i++ )
              out = compare_values(vx->data[i], vy->data[i], false, false);

            if ( out == 0 )
              out = 0 - (vx->cnt < vy->cnt) + (vx->cnt > vy->cnt);

            break;
          }

          case TABLE: {
            table_t* tx = as_table(x), * ty = as_table(y);
            usize maxc = MAX(tx->cnt, ty->cnt);
            
            for ( usize i=0; out == 0 && i < maxc; i++ ) {
              value_t k1 = tx->data[i*2], v1 = ty->data[i*2+1];
              value_t k2 = tx->data[i*2], v2 = ty->data[i*2+1];
                
              out = compare_values(k1, k2, false, false);
              
                if ( out == 0 )
                  out = compare_values(v1, v2, false, false);
            }
            
            if ( out == 0 )
              out = 0 - (tx->cnt < ty->cnt) + (tx->cnt > ty->cnt);

            break;
          }

          case NATIVE: {
            native_t* nx = as_native(x), * ny = as_native(y);
            out = compare_objects(nx->name, ny->name, false, false);

            break;
          }

          case CLOSURE: {
            closure_t* cx = as_closure(x), * cy = as_closure(y);
            out = compare_objects(cx->code, cy->code, false, false);

            if ( out == 0 )
              out = compare_objects(cx->envt, cy->envt, false, false);

            break;
          }

          case CHUNK: {
            chunk_t* cx = as_chunk(x), * cy = as_chunk(y);

            out = compare_objects(cx->name, cy->name, false, false);

            if ( out == 0 ) {
              out = compare_objects(cx->envt, cy->envt, false, false);

              if ( out == 0 ) {
                out = compare_objects(cx->vals, cy->vals, false, false);

                if ( out == 0 ) {
                  out = compare_objects(cx->code, cy->code, false, false);
                }
              }
            }

            break;
          }

          case NUMBER: {
            number_t nx = as_number(x), ny = as_number(y);
            out = CMP(nx, ny);

            break;
          }

          case POINTER: {
            pointer_t px = as_pointer(x), py = as_pointer(y);
            out = CMP(px, py);

            break;
          }

          case UNIT: {
            out = 0;

            break;
          }

          default: unreachable();
        }
      }
    }
  }

  return out;
}
