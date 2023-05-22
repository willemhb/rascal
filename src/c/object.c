#include "runtime.h"
#include "object.h"

// value APIs -----------------------------------------------------------------
object_t* as_object( value_t x );
usize size_of_type( type_t t );

type_t type_of( value_t x ) {
  switch ( x & TAGMASK ) {
    case OBJTAG:   return as_object(x)->type;
    case GLYPHTAG: return GLYPH;
    case NILTAG:   return UNIT;
    case PTRTAG:   return POINTER;
    default:       return NUMBER;
  }
}

#define size_of(x) _Generic((x), value_t: size_of_value, type_t: size_of_type)(x)

usize size_of_value( value_t x ) {
  return size_of_type( type_of(x) );
}

usize size_of_type( type_t t ) {
  switch ( t ) {
    case SYMBOL:      return sizeof(symbol_t);
    case LIST:        return sizeof(list_t);
    case BINARY:      return sizeof(binary_t);
    case VECTOR:      return sizeof(vector_t);
    case TABLE:       return sizeof(table_t);
    case NATIVE:      return sizeof(native_t);
    case CLOSURE:     return sizeof(closure_t);
    case CHUNK:       return sizeof(chunk_t);
    case NUMBER:      return sizeof(number_t);
    case GLYPH:       return sizeof(glyph_t);
    case POINTER:     return sizeof(pointer_t);
    case UNIT:        return sizeof(value_t);
    default:          return 0;
  }
}

// object APIs ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// object ---------------------------------------------------------------------
value_t object( void* p ) {
  return ((value_t)p) | OBJTAG;
}

void  init_object( void* obj, type_t type, flags fl ) {
  object_t* head = obj;
  head->next = Live;
  Live = head;
  head->type = type;
  head->flags = fl|GRAY;
}

void* make_object( type_t type, flags fl ) {
  object_t* out = allocate(size_of(type));
  init_object(out, type, fl);
  return out;
}

bool has_flag( void* ptr, flags fl ) {
  object_t* obj = ptr;
  return !!(obj->flags & fl);
}

// symbol ---------------------------------------------------------------------
symbol_t* as_symbol( value_t x ) {
  return (symbol_t*)(x & VALMASK);
}

bool is_symbol( value_t x ) {
  return type_of(x) == SYMBOL;
}

symbol_t* make_symbol( char* name, flags fl ) {
  assert(name);
  symbol_t* sym = make_object(SYMBOL, fl| FROZEN | (*name==':') * LITERAL);
  sym->idno     = ++SymbolCounter;
  sym->hash     = mix_3_hashes(hash_uword(SYMBOL), hash_uword(sym->idno), hash_str(name));
  sym->left     = NULL;
  sym->right    = NULL;
  sym->name     = duplicate_str(name);
  sym->bind     = UNDEFINED;

  return sym;
}

static symbol_t** locate_symbol(char* name, symbol_t** buf) {
  while (*buf) {
    int o = strcmp(name, (*buf)->name);

    if ( o < 0 )
      buf = &(*buf)->left;

    else if ( o > 0 )
      buf = &(*buf)->right;

    else
      break;
  }

  return buf;
}

static symbol_t* intern_symbol( char* name, flags fl ) {
  symbol_t** loc = locate_symbol(name, &SymbolTable);

  if (*loc == NULL)
    *loc = make_symbol(name, INTERNED|fl);

  return *loc;
}

symbol_t* symbol( char* name, bool interned ) {
  if ( interned )
    return intern_symbol(name, 0);

  return make_symbol(name, 0);
}

// list -----------------------------------------------------------------------
list_t* as_list( value_t x ) {
  return (list_t*)(x & VALMASK);
}

bool is_list( value_t x ) {
  return type_of(x) == LIST;
}

static void init_list(list_t* slf, value_t hd, list_t* tl) {
  slf->head  = hd;
  slf->tail  = tl;
  slf->arity = tl->arity+1;
}

list_t* cons(value_t hd, list_t* tl) {
  list_t* out = make_object(LIST, FROZEN);
  init_list(out, hd, tl);
  return out;
}

list_t* list(usize n, value_t* args) {
  list_t* out;
  if ( n == 0 )
    out = &EmptyList;

  else if ( n == 1 )
    out = cons(*args, &EmptyList);

  else {
    out = allocate(sizeof(list_t) * n);

    list_t* cur = &out[n-1], * prev = &EmptyList;

    for (usize i=n; i>0; i--, prev=cur, cur--) {
      init_object(cur, LIST, FROZEN);
      init_list(cur, args[i-1], prev);
    }
  }

  return out;
}

// binary ---------------------------------------------------------------------
bool is_binary( value_t x ) {
  return type_of(x) == BINARY;
}

binary_t* as_binary( value_t x ) {
  return (binary_t*)(x & VALMASK);
}

static void init_binary( binary_t* slf, int elSize, bool encoded ) {
  slf->elSize = elSize;
  slf->encoded = encoded;
  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

usize binary_size( binary_t* slf, bool cap ) {
  if ( cap )
    return slf->cap * slf->elSize;

  return (slf->cnt + slf->encoded) * slf->elSize;
}

usize resize_binary( binary_t* slf, usize n ) {
  usize an = n + slf->encoded;

  if ( an > slf->cap || an < (slf->cap >> 1)) {
    usize newc = n ? ceil2(an) : 0;
    slf->data = reallocate(slf->data, slf->cap*slf->elSize, newc*slf->elSize);
    slf->cap = newc;
  }

  slf->cnt = n;
  return slf->cnt;
}

void reset_binary( binary_t* slf ) {
  resize_binary(slf, 0);
}

void* binary_offset( binary_t* slf, usize n ) {
  return slf->data + (n * slf->elSize);
}

usize binary_write( binary_t* slf, usize n, void* data ) {
  usize oldCnt = slf->cnt;
  resize_binary(slf, slf->cnt+n);
  void* buf = binary_offset(slf, oldCnt);
  memcpy(buf, data, n * slf->elSize);
  return slf->cnt;
}

binary_t* binary( int elSize, bool encoded, usize n, void* data ) {
  binary_t* out = make_object(BINARY, 0);
  init_binary(out, elSize, encoded);

  if ( data )
    binary_write(out, n, data);

  return out;
}

// vector ---------------------------------------------------------------------
bool is_vector( value_t x ) {
  return type_of(x) == VECTOR;
}

vector_t* as_vector( value_t x ) {
  return (vector_t*)(x & VALMASK);
}

static void init_vector( vector_t* slf ) {
  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

usize vector_size( vector_t* slf, bool cap ) {
  return (cap ? slf->cap : slf->cnt) * sizeof(value_t);
}

usize resize_vector( vector_t* slf, usize n ) {
  if ( n > slf->cap || n < (slf->cap >> 1)) {
    usize newc = n ? ceil2(n) : 0;
    slf->data = reallocate(slf->data, slf->cap*sizeof(value_t), newc*sizeof(value_t));
    slf->cap = newc;
  }

  slf->cnt = n;
  return slf->cnt;
}

void reset_vector( vector_t* slf ) {
  resize_vector(slf, 0);
}

usize vector_write( vector_t* slf, usize n, value_t* data ) {
  usize oldCnt = slf->cnt;
  resize_vector(slf, slf->cnt+n);
  value_t* buf = slf->data + oldCnt;
  memcpy(buf, data, n*sizeof(value_t));
  return slf->cnt;
}

usize vector_push( vector_t* slf, value_t v ) {
  resize_vector(slf, slf->cnt+1);
  slf->data[slf->cnt-1] = v;
  return slf->cnt;
}

value_t vector_pop( vector_t* slf ) {
  assert(slf->cnt);
  value_t out = slf->data[slf->cnt-1];
  resize_vector(slf, slf->cnt-1);
  return out;
}

vector_t* vector( usize n, value_t* args ) {
  vector_t* out = make_object(VECTOR, 0);
  init_vector(out);
  if ( args )
    vector_write(out, n, args);
  return out;
}

// table ----------------------------------------------------------------------
#define MINCAP 8
#define LOADF 0.625

bool is_table( value_t x ) {
  return type_of(x) == TABLE;
}

table_t* as_table( value_t x ) {
  return (table_t*)(x & VALMASK);
}

uhash hash_value( value_t x, bool id );
uhash hash_object( void* ptr, bool id );
int compare_values( value_t x, value_t y, bool id, bool eq );
int compare_objects( void* x, void* y, bool id, bool eq );

static usize table_ord_size( usize cap ) {
  usize out;

  if ( cap < INT8_MAX )
    out = cap * sizeof(sint8);

  else if ( cap < INT16_MAX )
    out = cap * sizeof(sint16);

  else if ( cap < INT32_MAX )
    out = cap * sizeof(sint32);

  else
    out = cap * sizeof(sint64);

  return out;
}

static void* allocate_table_ords( usize cap ) {
  usize total = table_ord_size(cap); 
  void* out = allocate(total);
  memset(out, -1, total);
  return out;
}

static void deallocate_table_ords( void* data, usize cap ) {
  usize total = table_ord_size(cap);
  deallocate(data, total);
}

static void* reallocate_table_ords( void* data, usize oldCap, usize newCap ) {
  usize oldTotal = table_ord_size(oldCap), newTotal = table_ord_size(newCap);
  void* out = reallocate(data, oldTotal, newTotal);
  memset(out, -1, newTotal);

  return out;
}

static void init_table_data( value_t* data, usize cap ) {
  for ( usize i=0; i < cap; i++ ) {
    data[i*2]  = NOTFOUND;
    data[i*2+1]= NOTFOUND;
  }
}

static usize table_data_size( usize cap ) {
  return cap * 2 * sizeof(value_t);
}

static value_t* allocate_table_data( usize cap ) {
  return allocate(cap * 2 * sizeof(value_t));
}

static void deallocate_table_data( value_t* data, usize cap ) {
  deallocate(data, cap * 2 * sizeof(value_t));
}

static value_t* reallocate_table_data( value_t* oldData, usize oldCap, usize newCap ) {
  value_t* newData = reallocate(oldData, table_data_size(oldCap), table_data_size(newCap));

  if ( oldCap < newCap )
    init_table_data(newData+oldCap*2, newCap - oldCap);

  return newData;
}

static void init_table( table_t* slf ) {
  slf->cnt  = 0;
  slf->cap  = MINCAP;
  slf->data = allocate_table_data(slf->cap);
  slf->ord  = allocate_table_ords(slf->cap);
  init_table_data(slf->data, slf->cap);
}

static void free_table( table_t* slf ) {
  deallocate_table_data(slf->data, slf->cap);
  deallocate_table_ords(slf->ord, slf->cap);
}

static long get_table_ord( table_t* slf, usize n ) {
  long out;
  
  if ( slf->cap <= INT8_MAX )
    out = slf->ord8[n];

  else if ( slf->cap <= INT16_MAX )
    out = slf->ord16[n];

  else if ( slf->cap <= INT32_MAX )
    out = slf->ord32[n];

  else
    out = slf->ord64[n];

  return out;
}

static void set_table_ord( table_t* slf, usize n, usize o ) {
  if ( slf->cap <= INT8_MAX )
    slf->ord8[n] = o;

  else if ( slf->cap <= INT16_MAX )
    slf->ord16[n] = o;

  else if ( slf->cap <= INT32_MAX )
    slf->ord32[n] = o;

  else
    slf->ord64[n] = o;
}

usize table_size( table_t* slf, bool cap ) {
  return (cap ? slf->cap : slf->cnt) * 2 * sizeof(value_t);
}

usize resize_table( table_t* slf, usize n ) {
  if ( n > slf->cap * LOADF || n < (slf->cap >> 1) * LOADF) {
    usize newc = MAX((usize)MINCAP, ceil2(slf->cap));
    usize newm = newc - 1;
    bool isid = has_flag(slf, IDTABLE);
    slf->data = reallocate_table_data(slf->data, slf->cap, newc);
    slf->ord = reallocate_table_ords(slf->ord, slf->cap, newc);
    slf->cap = newc;

    if ( newc <= INT8_MAX ) {
      sint8* ord = slf->ord8;

      for ( usize i=0; i < slf->cnt; i++ ) {
        value_t k = slf->data[i*2];

        uhash h = hash_value(k, isid);
        usize j = h & newm;

        while ( ord[j] != -1 )
          j = (j + 1) & newm;

        ord[j] = i;
      }
    } else if ( newc <= INT16_MAX ) {
      sint16* ord = slf->ord16;

      for ( usize i=0; i < slf->cnt; i++ ) {
        value_t k = slf->data[i*2];

        uhash h = hash_value(k, isid);
        usize j = h & newm;

        while ( ord[j] != -1 )
          j = (j + 1) & newm;

        ord[j] = i;
      }      
    } else if ( newc <= INT32_MAX ) {
      sint32* ord = slf->ord32;

      for ( usize i=0; i < slf->cnt; i++ ) {
        value_t k = slf->data[i*2];

        uhash h = hash_value(k, isid);
        usize j = h & newm;

        while ( ord[j] != -1 )
          j = (j + 1) & newm;

        ord[j] = i;
      }
    } else {
      sint64* ord = slf->ord64;

      for ( usize i=0; i < slf->cnt; i++ ) {
        value_t k = slf->data[i*2];

        uhash h = hash_value(k, isid);
        usize j = h & newm;

        while ( ord[j] != -1 )
          j = (j + 1) & newm;

        ord[j] = i;
      }      
    }
  }

  slf->cnt = n;
  return slf->cnt;
}

static usize table_locate( table_t* slf, value_t k ) {
  bool id = has_flag(slf, IDTABLE);
  uhash h = hash_value(k, id);
  usize m = slf->cap-1;
  usize i = h & m;
  long n;

  if ( slf->cap < INT8_MAX ) {
    sint8* ord = slf->ord8;

    while ( (n=ord[i]) != -1 ) {
      if ( compare_values(k, slf->data[n*2], id, true) == 0 )
        break;

      i = (i + 1) & m;
    }
  } else if ( slf->cap < INT16_MAX ) {
    sint16* ord = slf->ord16;

    while ( (n=ord[i]) != -1 ) {
      if ( compare_values(k, slf->data[n*2], id, true) == 0 )
        break;

      i = (i + 1) & m;
    }
  } else if ( slf->cap < INT32_MAX ) {
    sint32* ord = slf->ord32;

    while ( (n=ord[i]) != -1 ) {
      if ( compare_values(k, slf->data[n*2], id, true) == 0 )
        break;

      i = (i + 1) & m;
    }
  } else {
    sint64* ord = slf->ord64;

    while ( (n=ord[i]) != -1 ) {
      if ( compare_values(k, slf->data[n*2], id, true) == 0 )
        break;

      i = (i + 1) & m;
    }
  }

  return i;
}

value_t table_get( table_t* slf, value_t k ) {
  usize n = table_locate(slf, k);
  long i = get_table_ord(slf, n);
  value_t v;

  if ( i == -1 )
    v = NOTFOUND;

  else
    v = slf->data[i*2+1];

  return v;
}

bool table_has( table_t* slf, value_t k ) {
  return table_get(slf, k) != NOTFOUND;
}

value_t table_set( table_t* slf, value_t k, value_t v ) {
  usize n = table_locate(slf, k);
  long i = get_table_ord(slf, n);

  if ( i == -1 ) {
    i = slf->cnt;
    slf->data[i*2] = k;
    resize_table(slf, slf->cnt+1);
  }

  slf->data[i*2+1] = v;

  return v;
}

value_t table_del( table_t* slf, value_t k ) {
  usize n = table_locate(slf, k);
  long i = get_table_ord(slf, n);
  value_t v;

  if ( i == -1 )
    v = NOTFOUND;

  else {
    v = slf->data[i*2+1];
    value_t* to = &slf->data[i*2];
    value_t* from = &slf->data[(i+1)*2];
    usize toMove = (slf->cnt - i - 1) * 2 * sizeof(value_t);
    memmove(to, from, toMove);
    slf->data[(slf->cnt-1)*2] = NOTFOUND;
    slf->data[(slf->cnt-1)*2+1] = NOTFOUND;
    set_table_ord(slf, n, -1);
    resize_table(slf, slf->cnt-1);
  }

  return v;
}

table_t* table( bool id, usize n, value_t* args ) {
  table_t* out = make_object(TABLE, id ? IDTABLE : EQTABLE);
  init_table(out);

  if ( args ) {
    assert((n & 1) == 0);

    for (usize i=0; i < n; i += 2)
      table_set(out, args[i], args[i+1]);
  }

  return out;
}

// native ---------------------------------------------------------------------
bool is_native( value_t x ) {
  return type_of(x) == NATIVE;
}

native_t* as_native( value_t x ) {
  return (native_t*)(x & VALMASK);
}

// closure --------------------------------------------------------------------
bool is_closure( value_t x ) {
  return type_of(x) == CLOSURE;
}

closure_t* as_closure( value_t x ) {
  return (closure_t*)(x & VALMASK);
}

// chunk ----------------------------------------------------------------------
bool is_chunk( value_t x ) {
  return type_of(x) == CHUNK;
}

chunk_t* as_chunk( value_t x ) {
  return (chunk_t*)(x & VALMASK);
}

// number ---------------------------------------------------------------------
bool is_number( value_t x ) {
  return type_of(x) == NUMBER;
}

number_t as_number( value_t x ) {
  return ((ieee64_t)x).dbl;
}

// glyph ----------------------------------------------------------------------
bool is_glyph( value_t x ) {
  return type_of(x) == GLYPH;
}

glyph_t as_glyph( value_t x ) {
  return (glyph_t)(x & VALMASK);
}

// pointer (internal only) ----------------------------------------------------
bool is_pointer( value_t x ) {
  return type_of(x) == POINTER;
}

pointer_t as_pointer( value_t x ) {
  return (pointer_t)(x & VALMASK);
}

// lang +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// hash -----------------------------------------------------------------------
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
