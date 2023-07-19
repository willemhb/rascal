#include <stdarg.h>
#include <tgmath.h>

#include "object.h"
#include "runtime.h"
#include "lang.h"

#include "util/hashing.h"
#include "util/string.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// cast/access/test functions -------------------------------------------------
number_t as_number( value_t x ) { return ((ieee64_t)x).dbl; }
glyph_t as_glyph( value_t x ) { return (glyph_t)(x & SMALLMASK); }
port_t as_port( value_t x ) { return (port_t)(x & VALMASK); }
native_t as_native( value_t x ) { return (native_t)(x & VALMASK); }
pointer_t as_pointer( value_t x ) { return (pointer_t)(x & VALMASK); }
object_t* as_object( value_t x ) { return (object_t*)(x & VALMASK); }
symbol_t* as_symbol( value_t x ) { return (symbol_t*)(x & VALMASK); }
list_t* as_list( value_t x ) { return (list_t*)(x & VALMASK); }
alist_t* as_alist( value_t x ) { return (alist_t*)(x & VALMASK); }
table_t* as_table( value_t x ) { return (table_t*)(x & VALMASK); }
buffer_t* as_buffer( value_t x ) { return (buffer_t*)(x & VALMASK); }
closure_t* as_closure( value_t x ) { return (closure_t*)(x & VALMASK); }
chunk_t* as_chunk( value_t x ) { return (chunk_t*)(x & VALMASK); }
control_t* as_control( value_t x ) { return (control_t*)(x & VALMASK); }
long as_integer( value_t x ) { return (long)as_number(x); }

bool is_number( value_t x ) { return (x & QNAN) != QNAN;  }
bool is_glyph( value_t x ) { return (x & WIDEMASK) == GLYPHTAG; }
bool is_port( value_t x ) { return (x & TAGMASK) == IOSTAG; }
bool is_native( value_t x ) { return (x & TAGMASK) == FUNTAG; }
bool is_pointer( value_t x ) { return (x & TAGMASK) == PTRTAG; }
bool is_unit( value_t x ) { return x == NIL; }
bool is_object( value_t x ) { return (x & TAGMASK) == OBJTAG; }
bool is_symbol( value_t x ) { return is_object(x) && as_object(x)->type == SYMBOL; }
bool is_list( value_t x ) { return is_object(x) && as_object(x)->type == LIST; }
bool is_alist( value_t x ) { return is_object(x) && as_object(x)->type == ALIST; }
bool is_table( value_t x ) { return is_object(x) && as_object(x)->type == TABLE; }
bool is_buffer( value_t x ) { return is_object(x) && as_object(x)->type == BUFFER; }
bool is_closure( value_t x ) { return is_object(x) && as_object(x)->type == CLOSURE; }
bool is_chunk( value_t x ) { return is_object(x) && as_object(x)->type == CHUNK; }
bool is_control( value_t x ) { return is_object(x) && as_object(x)->type == CONTROL; }
bool is_integer( value_t x ) { return is_number(x) && as_number(x) - as_integer(x) == 0; }

#define SAFECAST(type, ctype)                                       \
  ctype to_##type( value_t x, const char* fname )                   \
  {                                                                 \
    require(fname, is_##type(x), x, "expected a %s", #type);        \
    return as_##type(x);                                            \
  }

SAFECAST(number, number_t);
SAFECAST(glyph, glyph_t);
SAFECAST(port, port_t);
SAFECAST(native, native_t);
SAFECAST(pointer, pointer_t);
SAFECAST(symbol, symbol_t*);
SAFECAST(list, list_t*);
SAFECAST(alist, alist_t*);
SAFECAST(table, table_t*);
SAFECAST(buffer, buffer_t*);
SAFECAST(closure, closure_t*);
SAFECAST(chunk, chunk_t*);
SAFECAST(control, control_t*);

#undef SAFECAST

long to_integer( value_t x, const char* fname ) {
  require(fname, is_integer(x), x, "expected an integer");
  return as_integer(x);
}

bool object_hasfl( void* obj, flags fl ) {
  assert(obj);

  object_t* theObj = obj;
  bool out = !!(theObj->flags & fl);
  return out;
}

bool value_hasfl( value_t val, flags fl ) {
  assert(is_object(val));

  return object_hasfl(as_object(val), fl);
}

bool object_setfl( void* obj, flags fl ) {
  assert(obj);

  object_t* theObj = obj;
  bool out = !!(theObj->flags & fl);
  theObj->flags |= fl;
  return out;
}

bool value_setfl( value_t val, flags fl ) {
  assert(is_object(val));

  return object_setfl(as_object(val), fl);
}

// basic queries --------------------------------------------------------------
datatype_t type_of_value( value_t x ) {
  switch ( x & TAGMASK ) {
	case ATMTAG: return x >> 32 & UINT16_MAX;
	case FUNTAG: return NATIVE;
    case IOSTAG: return PORT;
    case PTRTAG: return POINTER;
	case OBJTAG: return as_object(x)->type;
	default:     return NUMBER;
  }
}

datatype_t type_of_object( void* obj ) {
  assert(obj);
  return ((object_t*)obj)->type;
}

usize size_of_value( value_t val ) {
  datatype_t dt = type_of(val);
  usize out = size_of_datatype(dt);

  return out;
}

usize size_of_object( void* obj ) {
  datatype_t dt = type_of(obj);
  usize out = size_of_datatype(dt);

  return out;
}

usize size_of_datatype( datatype_t dt ) {
  switch ( dt ) {
	case NUMBER:  return sizeof(number_t);
	case PORT:    return sizeof(port_t);
	case NATIVE:  return sizeof(native_t);
	case UNIT:    return sizeof(value_t);
	case SYMBOL:  return sizeof(symbol_t);
	case LIST:    return sizeof(list_t);
    case ALIST:   return sizeof(alist_t);
    case TABLE:   return sizeof(table_t);
    case BUFFER:  return sizeof(buffer_t);
	case CLOSURE: return sizeof(closure_t);
	case CHUNK:   return sizeof(chunk_t);
    case CONTROL: return sizeof(control_t);
	default:      return 0;
  }
}

// object lifetime APIs -------------------------------------------------------
void mark_value( value_t slf ) {
  if ( is_object(slf) )
	mark_object(as_object(slf));
}

void mark_object( void* obj ) {
  if ( obj == NULL )
	return;

  if ( ((object_t*)obj)->black )
	return;

  ((object_t*)obj)->black = true;

  push_gray(obj);
}

void trace_value( value_t val ) {
  assert(is_object(val));
  trace_object(as_object(val));
}

static void trace_symbol( symbol_t* s ) {
  mark(s->left);
  mark(s->right);
}

static void trace_list( list_t* l ) {
  mark(l->head);
  mark(l->tail);
}

static void trace_alist( alist_t* a ) {
  for ( usize i=0; i<a->cnt; i++ )
    mark(a->data[i]);
}

static void trace_table( table_t* t ) {
  for ( usize i=0; i<t->cap; i++ ) {
    if ( t->data[i*2] == NOTFOUND )
      continue;

    mark(t->data[i*2]);
    mark(t->data[i*2+1]);
  }
}

static void trace_chunk( chunk_t* ch ) {
  mark(ch->vars);
  mark(ch->code);
  mark(ch->vals);
}

static void trace_closure( closure_t* cl ) {
  mark(cl->code);
  mark(cl->envt);
}

static void trace_control( control_t* ctl ) {
  mark(ctl->frame.fn);

  for ( int i=0; i<ctl->fp; i++ )
    mark(ctl->frames[i].fn);

  for ( int i=0; i<ctl->sp; i++ )
    mark(ctl->values[i]);
}

void trace_object( void* obj ) {
  assert(obj);
  ((object_t*)obj)->gray = false;

  switch ( type_of(obj) ) {
	case SYMBOL:  trace_symbol(obj);  break;
	case LIST:    trace_list(obj);    break;
    case ALIST:   trace_alist(obj);   break;
    case TABLE:   trace_table(obj);   break;
    case CHUNK:   trace_chunk(obj);   break;
	case CLOSURE: trace_closure(obj); break;
    case CONTROL: trace_control(obj); break;
	default:                          break;
  }
}

static void destruct_symbol( symbol_t* s ) {
  deallocate(s->name, 0, false);
}

static void destruct_alist( alist_t* a ) {
  deallocate(a->data, 0, false);
}

static void destruct_table( table_t* t ) {
  deallocate(t->data, 0, false);
}

static void destruct_buffer( buffer_t* b ) {
  deallocate(b->data, 0, false);
}

void destruct_value( value_t val ) {
  assert(is_object(val));
  destruct_object(as_object(val));
}

void destruct_object( void* obj ) {
  if ( obj == NULL )
    return;

  if ( !hasfl(obj, NOFREE) )
    switch ( type_of(obj) ) {
      case SYMBOL: destruct_symbol(obj); break;
      case ALIST:  destruct_alist(obj);  break;
      case TABLE:  destruct_table(obj);  break;
      case BUFFER: destruct_buffer(obj); break;
      default:                           break;
    }

  if ( !hasfl(obj, NODEALLOC) )
    deallocate(obj, size_of(obj), true);
}

// high level constructors ----------------------------------------------------
value_t number( number_t num ) {
  return ((ieee64_t)num).word;
}

value_t port( port_t p ) {
  return ((value_t)p) | IOSTAG;
}

value_t native( native_t n ) {
  return ((value_t)n) | FUNTAG;
}

value_t pointer( pointer_t p ) {
  return ((value_t)p) | PTRTAG;
}

value_t object( void* obj ) {
  return ((value_t)obj) | OBJTAG;
}

static void init_object( void* obj, datatype_t type, flags fl ) {
  object_t* slf = obj;

  slf->type  = type;
  slf->next  = Vm.live;
  slf->hash  = 0;
  slf->flags = fl;
  slf->gray  = true;
  slf->black = false;

  Vm.live = slf;
}

static symbol_t** find_symbol( char* name, symbol_t** st ) {
  while ( *st ) {
    int o = strcmp(name, (*st)->name);

    if ( o < 0 )
      st = &(*st)->left;

    else if ( o > 0 )
      st = &(*st)->right;

    else
      break;
  }

  return st;
}

static symbol_t* make_symbol( char* name ) {
  symbol_t* out = allocate(sizeof(symbol_t), true);
  init_object(out, SYMBOL, FROZEN);
  out->left = out->right = NULL;
  out->idno = ++Vm.symbolCounter;
  out->name = duplicates(name, false);

  return out;
}

symbol_t* symbol( char* name ) {
  symbol_t** loc = find_symbol(name, &Vm.symbolTable);

  if ( *loc == NULL )
    *loc = make_symbol(name);

  return *loc;
}

symbol_t* gensym( char* name ) {
  if ( name == NULL )
    name = "symbol";

  return make_symbol(name);
}

// alist API
void init_alist( alist_t* slf, bool ini ) {
  if ( ini )
    init_object(slf, ALIST, 0);

  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

void free_alist( alist_t* slf ) {
  deallocate(slf->data, 0, false);
}

void reset_alist( alist_t* slf ) {
  free_alist(slf);
  init_alist(slf, false);
}

usize resize_alist( alist_t* slf, usize n ) {
  if ( n > slf->cap || n < (slf->cap >> 1) ) {
    slf->cap = n ? ceil2(n) : 0;
    slf->data = reallocate(slf->data, 0, slf->cap * sizeof(value_t), false);
  }

  slf->cnt = n;
  return slf->cnt;
}

usize alist_push( alist_t* slf, value_t val ) {
  resize_alist(slf, slf->cnt+1);
  slf->data[slf->cnt-1] = val;
  return slf->cnt-1;
}

value_t alist_pop( alist_t* slf ) {
  value_t out = slf->data[slf->cnt-1];
  resize_alist(slf, slf->cnt-1);
  return out;
}

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define LOAD_FACTOR 0.625
#define LOAD_PAD 1.6
#define MIN_CAP 8

// simple hash table type +++++++++++++++++++++++++++++++++++++++++++++++++++++
// misc utilities -------------------------------------------------------------
static value_t* allocate_table_data( usize cap ) {
  value_t* data = allocate(cap * 2 * sizeof(value_t), false);
  for ( usize i=0; i<cap; i++ )
    data[i*2] = data[i*2+1] = NOTFOUND;
  return data;
}

static void rehash_table( table_t* slf, usize newCap, value_t* newData ) {
  usize oldCap = slf->cap;
  value_t* oldData = slf->data;
  usize newMask = newCap - 1;

  for ( usize i=0; i<oldCap; i++ ) {
    value_t key = oldData[i*2], val = oldData[i*2+1];

    if ( key == NOTFOUND )
      continue;

    uhash keyHash = hash(key);
    uhash j = keyHash & newMask;

    while ( newData[j*2] != NOTFOUND )
      j = (j + 1) & newMask;

    newData[j*2] = key;
    newData[j*2+1] = val;
  }
}

void init_table( table_t* slf, bool ini ) {
  if ( ini )
    init_object(slf, TABLE, 0);

  slf->cnt = 0;
  slf->cap = MIN_CAP;
  slf->data = allocate_table_data(slf->cap);
}

void free_table( table_t* slf ) {
  deallocate(slf->data, 0, false);
}

void reset_table( table_t* slf ) {
  free_table(slf);
  init_table(slf, false);
}

usize resize_table( table_t* slf, usize n ) {
  usize pn = ceil(n * LOAD_PAD);
  
  if ( pn > slf->cap || pn < (slf->cap >> 1) ) {
    usize newCap = pn <= MIN_CAP ? MIN_CAP : ceil2(pn);
    value_t* newData = allocate_table_data(newCap);
    rehash_table(slf, newCap, newData);
    slf->cap = newCap;
    slf->data = newData;
  }

  return slf->cap;
}

value_t* table_find( table_t* slf, value_t key ) {
  usize cap = slf->cap;
  value_t* data = slf->data;
  usize mask = cap-1;
  usize keyHash = hash(key);
  usize i = keyHash & mask;

  while ( data[i*2] != NOTFOUND && equal(key, data[i*2]) )
    i = (i + 1) & mask;

  return &data[i*2];
}

value_t table_get( table_t* slf, value_t k ) {
  value_t* buf = table_find(slf, k);
  return buf[1];
}

value_t table_add( table_t* slf, value_t k, value_t v ) {
  resize_table(slf, slf->cnt+1);

  value_t* buf = table_find(slf, k);
  value_t out = buf[1];

  if ( *buf == NOTFOUND ) {
    slf->cnt++;
    buf[0] = k;
    buf[1] = v;
  }

  return out;
}

value_t table_set( table_t* slf, value_t k, value_t v ) {
  resize_table(slf, slf->cnt+1);

  value_t* buf = table_find(slf, k);
  value_t out = buf[1];

  if ( *buf == NOTFOUND ) {
    slf->cnt++;
    *buf = k;
  }

  buf[1] = v;
  return out;
}

value_t table_del( table_t* slf, value_t k ) {
  value_t* buf = table_find(slf, k);
  value_t out = buf[1];

  if ( *buf != NOTFOUND ) {
    buf[0] = NOTFOUND;
    buf[1] = NOTFOUND;
    slf->cnt--;
    resize_table(slf, slf->cnt);
  }

  return out;
}

// buffer API
void init_buffer( buffer_t* slf, bool ini, ... ) {
  if ( ini ) {
    init_object(slf, BUFFER, 0);
    va_list va;
    va_start(va, ini);
    slf->elSize   = va_arg(va, int);
    slf->encoding = va_arg(va, encoding_t);
    va_end(va);
  }
  slf->cnt  = 0;
  slf->cap  = 0;
  slf->data = NULL;
}

void free_buffer( buffer_t* slf ) {
  deallocate(slf->data, 0, false);
}

void reset_buffer( buffer_t* slf ) {
  free_buffer(slf);
  init_buffer(slf, false);
}

usize resize_buffer( buffer_t* slf, usize n ) {
  usize pn = n + !!slf->encoding;

  if ( pn > slf->cap || pn < (slf->cap >> 1) ) {
    slf->cap = n ? ceil2(pn) : 0;
    slf->data = reallocate(slf->data, 0, slf->cap * slf->elSize, false);
  }

  slf->cnt = n;
  return slf->cnt;
}

usize buffer_write( buffer_t* slf, usize n, void* data ) {
  usize off = slf->cnt * slf->elSize;
  usize nw = n * slf->elSize;
  resize_buffer(slf, slf->cnt + n);
  memcpy(slf->data + off, data, nw);
  return slf->cnt - n;
}

// initialization -------------------------------------------------------------
void toplevel_init_object( void ) {}
