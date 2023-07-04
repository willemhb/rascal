#ifndef object_h
#define object_h

#include "common.h"

#include "util/number.h"
#include "util/buffer.h"
#include "util/table.h"
#include "util/alist.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum datatype {
  NOTYPE, // not a datatype
  NUMBER=1,
  PORT,
  NATIVE,
  UNIT,

  OBJECT=UNIT,
  SYMBOL,
  LIST,
  TUPLE,
  CLOSURE,
  CHUNK
};

#define NDTYPES (CONTROL+1)

struct object {
  object_t* next; // invasive linked list of live objects
  uhash hash;
  datatype_t type;
  ushort flags;
  ubyte black;
  ubyte gray;
};

#define HEADER object_t obj

// user object types
struct symbol {
  HEADER;
  symbol_t* left, * right;
  char*   name;
  uint64  idno;
};

struct list {
  HEADER;
  usize arity;
  value_t head;
  list_t* tail;
};

struct tuple {
  HEADER;
  usize arity;
  value_t slots[];
};

struct closure {
  HEADER;
  chunk_t* code;
  tuple_t* envt;
};

struct chunk {
  HEADER;
  values_t vals;
  buffer_t instr;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// object flags ---------------------------------------------------------------
enum {
  HASHED=0x8000
};

// empty singletons -----------------------------------------------------------
extern list_t EmptyList;
extern tuple_t EmptyTuple;

// tags -----------------------------------------------------------------------
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define NUMTAG   0x0000000000000000ul // dummy
#define IOSTAG   0x7ffd000000000000ul
#define NILTAG   0x7ffe000000000000ul
#define NTVTAG   0x7fff000000000000ul
#define OBJTAG   0xffff000000000000ul

#define TAGMASK   0xffff000000000000ul
#define VALMASK   0x0000fffffffffffful

#define NIL       (NILTAG|0)
#define NOTFOUND  (NILTAG|1)
#define UNDEFINED (NILTAG|3)
#define UNBOUND   (NILTAG|5)

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// cast/access/test functions -------------------------------------------------
number_t as_number( value_t x );
void* as_pointer( value_t x );
port_t as_port( value_t x );
native_t as_native( value_t x );
object_t* as_object( value_t x );
symbol_t* as_symbol( value_t x );
list_t* as_list( value_t x );
tuple_t* as_tuple( value_t x );
closure_t* as_closure( value_t x );
chunk_t* as_chunk( value_t x );

bool is_number( value_t x );
bool is_port( value_t x );
bool is_native( value_t x );
bool is_unit( value_t x );
bool is_object( value_t x );
bool is_symbol( value_t x );
bool is_list( value_t x );
bool is_closure( value_t x );
bool is_chunk( value_t x );

long intval( value_t x );
uword wrdval( value_t x );

#define hasfl( x, fl ) (!!(as_object(x)->flags & (fl)))
#define setfl( x, fl ) (as_object(x)->flags |= (fl))

#define sethash( x, h )							 \
  do {                                           \
    value_t _x = x;                              \
    as_object(_x)->hash = h;                     \
    as_object(_x)->flags |= HASHED;              \
  } while (false)

// basic queries --------------------------------------------------------------
#define type_of( x ) generic((x), value_t:type_of_value, default:type_of_object)(x)
datatype_t type_of_value( value_t val );
datatype_t type_of_object( void* obj );

#define size_of( x )                            \
  generic((x),                                  \
          value_t:size_of_value,                \
          datatype_t:size_of_datatype,          \
          default:size_of_object)(x)

usize size_of_value( value_t x );
usize size_of_object( void* obj );
usize size_of_datatype( datatype_t dt );

// object lifetime APIs -------------------------------------------------------
#define mark( x ) generic((x), value_t:mark_value, default:mark_object)(x)
void mark_value( value_t val );
void mark_object( void* obj );

#define trace( x ) generic((x), value_t:trace_value, default: trace_object)(x)
void trace_value( value_t val );
void trace_object( void* obj );

#define destruct( x ) generic((x), value_t:destruct_value, default: destruct_object)(x)
void destruct_value( value_t val );
void destruct_object( void* obj );

// high level constructors ----------------------------------------------------
value_t number( number_t n );
value_t pointer( void* p );
value_t port( port_t p );
value_t native( native_t n );
value_t object( void* o );
symbol_t* symbol( char* name );
symbol_t* gensym( char* name );
list_t* list( value_t head, list_t* tail );
tuple_t* tuple( usize n, ... );
closure_t* closure( chunk_t* chunk, usize n, value_t vals );
chunk_t* chunk( void );

// initialization -------------------------------------------------------------
void toplevel_init_object( void );

#endif
