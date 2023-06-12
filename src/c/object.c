#include "object.h"
#include "runtime.h"

#include "util/number.h"
#include "util/hashing.h"
#include "util/string.h"

// value APIs -----------------------------------------------------------------
object_t* as_object( value_t x );
usize size_of_type( type_t t );

type_t type_of( value_t x ) {
  switch ( x & TAGMASK ) {
    case GLYPHTAG: return GLYPH;
    case NILTAG:   return UNIT;
    case IOSTAG:   return PORT;
    case PTRTAG:   return POINTER;
    case OBJTAG:   return as_object(x)->type;
    default:       return NUMBER;
  }
}

usize size_of_value( value_t x ) {
  return size_of_type( type_of(x) );
}

usize size_of_type( type_t t ) {
  switch ( t ) {
    case SYMBOL:      return sizeof(symbol_t);
    case PAIR:        return sizeof(pair_t);
    case CONS:        return sizeof(cons_t);
    case BINARY:      return sizeof(binary_t);
    case VECTOR:      return sizeof(vector_t);
    case TABLE:       return sizeof(table_t);
    case RECORD:      return sizeof(record_t);
    case NATIVE:      return sizeof(native_t);
    case CHUNK:       return sizeof(chunk_t);
    case CLOSURE:     return sizeof(closure_t);
    case CONTROL:     return sizeof(control_t);
    case NUMBER:      return sizeof(number_t);
    case GLYPH:       return sizeof(glyph_t);
    case PORT:        return sizeof(port_t);
    case UNIT:        return sizeof(value_t);
    case POINTER:     return sizeof(pointer_t); 
    default:          return 0;
  }
}

// object APIs ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// object ---------------------------------------------------------------------
value_t object( void* p ) { 
  return p ? ((value_t)p) | OBJTAG : NIL;
}

void  init_object( void* obj, type_t type, flags fl ) {
  object_t* head = obj;

  if ( !!(fl & ISALLOC) ) {
    head->next = Heap.live;
    Heap.live  = head;
  }

  head->type = type;
  head->flags = fl|GRAY;
}

void* make_object( type_t type, flags fl ) {
  object_t* out = allocate(size_of(type)) + sizeof(object_t*);
  init_object(out, type, fl|ISALLOC);
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
  symbol_t* sym = make_object(SYMBOL, fl| ISFROZEN | (*name==':') * LITERAL);
  sym->idno     = ++SymbolTable.counter;
  sym->hash     = mix_3_hashes(hash_uword(SYMBOL), hash_uword(sym->idno), hash_str(name));
  sym->left     = NULL;
  sym->right    = NULL;
  sym->name     = duplicate_str(name);
  sym->constant = UNDEFINED;

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
  symbol_t** loc = locate_symbol(name, &SymbolTable.root);

  if (*loc == NULL)
    *loc = make_symbol(name, INTERNED|fl);

  return *loc;
}

symbol_t* symbol( char* name, bool intern ) {
  if ( intern )
    return intern_symbol(name, 0);

  return make_symbol(name, 0);
}

// cons -----------------------------------------------------------------------
cons_t* as_cons( value_t x ) {
  return (cons_t*)(x & VALMASK);
}

bool is_cons( value_t x ) {
  return type_of(x) == CONS;
}

static void init_cons(cons_t* slf, value_t hd, cons_t* tl) {
  slf->head  = hd;
  slf->tail  = tl;
  slf->arity = 1 + (tl ? tl->arity : 0);
}

usize list_arity( cons_t* slf ) {
  return slf ? slf->arity : 0;
}

value_t list_head( cons_t* slf ) {
  return slf ? slf->head : NIL;
}

cons_t* list_tail( cons_t* slf ) {
  return slf ? slf->tail : NULL;
}

cons_t* cons( value_t hd, cons_t* tl ) {
  cons_t* out = make_object(CONS, ISFROZEN);
  init_cons(out, hd, tl);
  return out;
}

cons_t* list( usize n, value_t* args ) {
  cons_t* out;
  if ( n == 0 )
    out = NULL;

  else if ( n == 1 )
    out = cons(*args, NULL);

  else {
    out = allocate(sizeof(cons_t) * n);

    cons_t* cur = &out[n-1], * prev = NULL;

    for (usize i=n; i>0; i--, prev=cur, cur--) {
      init_object(cur, CONS, ISFROZEN);
      init_cons(cur, args[i-1], prev);
    }
  }

  return out;
}

cons_t* consn( usize n, value_t* args ) {
  cons_t* out;
  
  if ( n == 2 )
    out = cons(args[0], as_cons(args[1]));

  else {
    out = allocate(sizeof(cons_t) * n-1);

    cons_t* cur = &out[n-2], * prev = as_cons(args[n-1]);

    for ( usize i=n-1; i>0; i--, prev=cur, cur-- ) {
      init_object(cur, CONS, ISFROZEN);
      init_cons(cur, args[i-1], prev);
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

bool is_encoded( binary_t* slf ) {
  return has_flag(slf, ISENCODED);
}

usize elsize( binary_t* slf ) {
  return slf->obj.flags & UINT8_MAX;
}

static void init_binary( binary_t* slf ) {
  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

usize binary_size( binary_t* slf, bool cap ) {
  if ( cap )
    return slf->cap * elsize(slf);

  else
    return (slf->cnt + is_encoded(slf)) * elsize(slf);
}

usize resize_binary( binary_t* slf, usize n ) {
  usize an = n + is_encoded(slf);

  if ( an > slf->cap || an < (slf->cap >> 1)) {
    usize newc = n ? ceil2(an) : 0;
    usize es = elsize(slf);
    slf->data = reallocate(slf->data, slf->cap*es, newc*es);
    slf->cap = newc;
  }

  slf->cnt = n;
  return slf->cnt;
}

void reset_binary( binary_t* slf ) {
  resize_binary(slf, 0);
}

void* binary_offset( binary_t* slf, usize n ) {
  return slf->data + (n * elsize(slf));
}

usize binary_write( binary_t* slf, usize n, void* data ) {
  usize oldCnt = slf->cnt;
  resize_binary(slf, slf->cnt+n);
  void* buf = binary_offset(slf, oldCnt);
  memcpy(buf, data, n * elsize(slf));
  return slf->cnt;
}

binary_t* binary( int elSize, bool encoded, usize n, void* data ) {
  binary_t* out = make_object(BINARY, encoded*ISENCODED | elSize);
  init_binary(out);

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

usize reset_vector( vector_t* slf ) {
  return resize_vector(slf, 0);
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
#define LOADC 1.6   // complement to load factor

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

static usize pad_table_ords_size( usize newn, usize oldc ) {
  usize newc = oldc;
  usize loaded = newc * LOADF;

  if ( newn > loaded || newn < (loaded >> 1) )
    newc = ceil2(newn * LOADC);

  return newc;
}

static usize pad_table_data_size( usize newn, usize oldn, usize oldc ) {
  usize newc = oldc;
  
  if ( newn > oldc || newn < (oldc >> 1) ) {
    newc = (newn + (newn >> 3) + 6) & ~3ul;

    if ( newn - oldn > newc - newn )
      newc = (newn + 3) & ~3ul;
  }

  return newc;
}

static pair_t** allocate_table_data( usize cap ) {
  return allocate(cap * sizeof(pair_t*));
}

static void deallocate_table_data( pair_t** data, usize cap ) {
  deallocate(data, cap * sizeof(pair_t*));
}

static pair_t** reallocate_table_data( pair_t** oldData, usize oldCap, usize newCap ) {
  usize oldSize    = oldCap * sizeof(pair_t*);
  usize newSize    = newCap * sizeof(pair_t*);
  pair_t** newData = reallocate(oldData, oldSize, newSize);

  return newData;
}

static void init_table( table_t* slf ) {
  slf->cnt  = 0;
  slf->dcap = 0;
  slf->ocap = MINCAP;
  slf->data = allocate_table_data(slf->dcap);
  slf->ord  = allocate_table_ords(slf->ocap);
}

static void free_table( table_t* slf ) {
  deallocate_table_data(slf->data, slf->dcap);
  deallocate_table_ords(slf->ord, slf->ocap);
}

static long get_table_ord( table_t* slf, usize n ) {
  long out;
  
  if ( slf->ocap <= INT8_MAX )
    out = slf->ord8[n];

  else if ( slf->ocap <= INT16_MAX )
    out = slf->ord16[n];

  else if ( slf->ocap <= INT32_MAX )
    out = slf->ord32[n];

  else
    out = slf->ord64[n];

  return out;
}

static void set_table_ord( table_t* slf, usize n, usize o ) {
  if ( slf->ocap <= INT8_MAX )
    slf->ord8[n] = o;

  else if ( slf->ocap <= INT16_MAX )
    slf->ord16[n] = o;

  else if ( slf->ocap <= INT32_MAX )
    slf->ord32[n] = o;

  else
    slf->ord64[n] = o;
}

usize table_size( table_t* slf, bool cap ) {
  return (cap ? slf->ocap : slf->cnt) * 2 * sizeof(value_t);
}

usize resize_table( table_t* slf, usize n ) {
  usize newd = pad_table_data_size(n, slf->cnt, slf->dcap);
  
  if ( newd != slf->dcap ) {
    slf->data = reallocate_table_data(slf->data, slf->dcap, newd);
    slf->dcap = newd;
  }

  usize newo = pad_table_ords_size(n, slf->ocap);

  if ( newo != slf->ocap ) {
    usize newm = newo - 1;
    
    slf->ord  = reallocate_table_ords(slf->ord, slf->ocap, newo);
    slf->ocap = newo;

    if ( n > 0 ) {
      if ( newo <= INT8_MAX ) {
        sint8* ord = slf->ord8;

        for ( usize i=0; i < slf->cnt; i++ ) {
          pair_t* kv = slf->data[i];
          uhash h    = kv->hash;
          usize j    = h & newm;

          while ( ord[j] != -1 )
            j = (j + 1) & newm;

          ord[j] = i;
        }
      } else if ( newo <= INT16_MAX ) {
        sint16* ord = slf->ord16;

        for ( usize i=0; i < slf->cnt; i++ ) {
          pair_t* kv = slf->data[i];
          uhash h    = kv->hash;
          usize j    = h & newm;
          
          while ( ord[j] != -1 )
            j = (j + 1) & newm;

          ord[j] = i;
        }      
      } else if ( newo <= INT32_MAX ) {
        sint32* ord = slf->ord32;

        for ( usize i=0; i < slf->cnt; i++ ) {
          pair_t* kv = slf->data[i];
          uhash h    = kv->hash;
          usize j    = h & newm;
          
          while ( ord[j] != -1 )
            j = (j + 1) & newm;

          ord[j] = i;
        }
      } else {
        sint64* ord = slf->ord64;

        for ( usize i=0; i < slf->cnt; i++ ) {
          pair_t* kv = slf->data[i];
          uhash h    = kv->hash;
          usize j    = h & newm;
          
          while ( ord[j] != -1 )
            j = (j + 1) & newm;

          ord[j] = i;
        }      
      }
    }
  }

  slf->cnt = n;
  return slf->cnt;
}

usize reset_table( table_t* slf ) {
  return resize_table(slf, 0);
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

value_t number( number_t x ) {
  return ((ieee64_t)x).word;
}

// glyph ----------------------------------------------------------------------
bool is_glyph( value_t x ) {
  return type_of(x) == GLYPH;
}

glyph_t as_glyph( value_t x ) {
  return (glyph_t)(x & VALMASK);
}

value_t glyph( int ch ) {
  return (value_t)ch | GLYPHTAG;
}

// pointer (internal only) ----------------------------------------------------
bool is_pointer( value_t x ) {
  return type_of(x) == POINTER;
}

pointer_t as_pointer( value_t x ) {
  return (pointer_t)(x & VALMASK);
}

value_t pointer( pointer_t x ) {
  return (value_t)x | PTRTAG;
}

// unit -----------------------------------------------------------------------
bool is_unit( value_t x ) {
  return x == NIL;
}
