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

// equal implementation -------------------------------------------------------
static bool equal_lists( list_t* lx, list_t* ly ) {
  bool out = lx->arity == ly->arity;

  for ( ; out && lx->arity; lx=lx->tail, ly=ly->tail )
    out = equal(lx->head, ly->head);

  return out;
}

static bool equal_alists( alist_t* ax, alist_t* ay ) {
  bool out = ax->cnt == ay->cnt;

  for ( usize i=0; out && i<ax->cnt; i++ )
    out = equal(ax->data[i], ay->data[i]);

  return out;
}

static bool equal_tables( table_t* tx, table_t* ty ) {
  bool out = tx->cnt == ty->cnt;

  for ( usize i=0; out && i<tx->cap; i++ ) {
    out = equal(tx->data[i*2], ty->data[i*2]);
    out = out && equal(tx->data[i*2+1], ty->data[i*2+1]);
  }

  return out;
}

static bool equal_buffers( buffer_t* bx, buffer_t* by ) {
  bool out = bx->cnt == by->cnt;
  out = out && bx->encoding == by->encoding;
  out = out && bx->elSize == by->elSize;
  out = out && memcmp(bx->data, by->data, bx->cnt * bx->elSize == 0);
  return out;
}

static bool equal_chunks( chunk_t* cx, chunk_t* cy ) {
  bool out = equal_alists(cx->vars, cy->vars) &&
    equal_alists(cx->vals, cy->vals) &&
    equal_buffers(cx->code, cy->code);

  return out;
}

static bool equal_closures( closure_t* cx, closure_t* cy ) {
  bool out = equal_chunks(cx->code, cy->code) && equal_alists(cx->envt, cy->envt);

  return out;
}

bool equal( value_t x, value_t y ) {
  bool out;

  if ( x == y )
    out = true;

  if ( is_object(x) ) {
    datatype_t tx = type_of(x), ty = type_of(y);

    if ( tx == ty )
      switch ( tx ) {
        case LIST:    out = equal_lists(as_list(x), as_list(y)); break;
        case ALIST:   out = equal_alists(as_alist(x), as_alist(y)); break;
        case TABLE:   out = equal_tables(as_table(x), as_table(y)); break;
        case BUFFER:  out = equal_buffers(as_buffer(x), as_buffer(y)); break;
        case CHUNK:   out = equal_chunks(as_chunk(x), as_chunk(y)); break;
        case CLOSURE: out = equal_closures(as_closure(x), as_closure(y)); break;
        default:      out = false; break;
      }
  } else {
    out = false;
  }

  return out;
}

// compare implementation -----------------------------------------------------
static int compare_symbols( symbol_t* sx, symbol_t* sy ) {
  int out = strcmp(sx->name, sy->name);
  out = out ? : CMP(sx->idno, sy->idno);

  return out;
}

static int compare_lists( list_t* lx, list_t* ly ) {
  int out = 0;

  for ( ; out == 0 && lx->arity && ly->arity; lx=lx->tail, ly=ly->tail )
    out = compare(lx->head, ly->head);

  out = out ? : 0 - (lx->arity < ly->arity) + (ly->arity < lx->arity);

  return out;
}

int compare( value_t x, value_t y ) {
  int out = 0;

  if ( x == y )
    out = 0;

  
  
  return out;
}

// initialization -------------------------------------------------------------
void toplevel_init_lang( void );
