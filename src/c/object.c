#include <stdarg.h>

#include "object.h"
#include "runtime.h"

#include "util/hashing.h"
#include "util/string.h"


// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// cast/access/test functions -------------------------------------------------
number_t as_number( value_t x ) {
  return ((ieee64_t)x).dbl;
}

port_t as_port( value_t x ) {
  return (port_t)(x & VALMASK);
}

native_t as_native( value_t x ) {
  return (native_t)(x & VALMASK);
}

object_t* as_object( value_t x ) {
  return (object_t*)(x & VALMASK);
}

symbol_t* as_symbol( value_t x ) {
  return (symbol_t*)(x & VALMASK);
}

list_t* as_list( value_t x ) {
  return (list_t*)(x & VALMASK);
}

tuple_t* as_tuple( value_t x ) {
  return (tuple_t*)(x & VALMASK);
}

closure_t* as_closure( value_t x ) {
  return (closure_t*)(x & VALMASK);
}

chunk_t* as_chunk( value_t x ) {
  return (chunk_t*)(x & VALMASK);
}

bool is_number( value_t x ) {
  return (x & QNAN) != QNAN; 
}

bool is_port( value_t x ) {
  return (x & TAGMASK) == IOSTAG;
}

bool is_native( value_t x ) {
  return (x & TAGMASK) == NTVTAG;
}

bool is_unit( value_t x ) {
  return x == NIL;
}

bool is_object( value_t x ) {
  return (x & TAGMASK) == OBJTAG;
}

bool is_symbol( value_t x ) {
  return is_object(x) && as_object(x)->type == SYMBOL;
}

bool is_list( value_t x ) {
  return is_object(x) && as_object(x)->type == LIST;
}

bool is_tuple( value_t x ) {
  return is_object(x) && as_object(x)->type == TUPLE;
}

bool is_closure( value_t x ) {
  return is_object(x) && as_object(x)->type == CLOSURE;
}

bool is_chunk( value_t x ) {
  return is_object(x) && as_object(x)->type == CHUNK;
}

long intval( value_t x ) {
  return (long)as_number(x);
}

uword wrdval( value_t x ) {
  return (uword)as_number(x);
}

void* ptrval( value_t x ) {
  return (void*)wrdval(x);
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
	case IOSTAG: return PORT;
	case NILTAG: return UNIT;
	case NTVTAG: return NATIVE;
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

  if ( dt == TUPLE )
    out += sizeof(value_t) * as_tuple(val)->arity;

  return out;
}

usize size_of_object( void* obj ) {
  datatype_t dt = type_of(obj);
  usize out = size_of_datatype(dt);

  if ( dt == TUPLE )
    out += sizeof(value_t) * ((tuple_t*)obj)->arity;

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
    case TUPLE:   return sizeof(tuple_t);
	case CLOSURE: return sizeof(closure_t);
	case CHUNK:   return sizeof(chunk_t);
	default:      return 0;
  }
}

// object lifetime APIs -------------------------------------------------------
static void push_gray( void* obj ) {
  objects_push(&Vm.grays, obj);
}

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

void trace_object( void* obj ) {
  assert(obj);
  ((object_t*)obj)->gray = false;

  switch ( type_of(obj) ) {
	case SYMBOL:
	  mark(((symbol_t*)obj)->left);
	  mark(((symbol_t*)obj)->right);
	  break;

	case LIST:
	  mark(((list_t*)obj)->head);
	  mark(((list_t*)obj)->tail);
	  break;

	case TUPLE:
	  for ( usize i=0; i<((tuple_t*)obj)->arity; i++ )
		mark(((tuple_t*)obj)->slots[i]);
	  break;

	case CLOSURE:
	  mark(((closure_t*)obj)->code);
	  mark(((closure_t*)obj)->envt);
	  break;

	case CHUNK:
	  trace_values(&((chunk_t*)obj)->vals);
	  break;

	default:
	  break;
  }
}

void destruct_value( value_t val ) {
  assert(is_object(val));
  destruct_object(as_object(val));
}

void destruct_object( void* obj ) {
  if ( obj == NULL )
    return;

  switch ( type_of(obj) ) {
    case SYMBOL:
      deallocate(((symbol_t*)obj)->name, 0, false);
      break;

    case CHUNK:
      free_values(&((chunk_t*)obj)->vals);
      free_buffer(&((chunk_t*)obj)->instr);
      break;

    default:
      break;
  }

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
  return ((value_t)n) | NTVTAG;
}

value_t object( void* obj ) {
  return ((value_t)obj) | OBJTAG;
}

static void init_object( void* obj, datatype_t type ) {
  object_t* slf = obj;

  slf->type  = type;
  slf->next  = Vm.live;
  slf->hash  = 0;
  slf->flags = 0;
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
  init_object(out, SYMBOL);
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

list_t* list( value_t head, list_t* tail ) {
  
}

tuple_t* tuple( usize n, ... );
closure_t* closure( chunk_t* chunk, usize n, value_t vals );
chunk_t* chunk( void );

// initialization -------------------------------------------------------------
void toplevel_init_object( void ) {}
