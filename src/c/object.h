#ifndef object_h
#define object_h

#include "common.h"

#include "util/number.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum datatype {
  NOTYPE, // not a datatype
  NUMBER=1,
  GLYPH,
  UNIT,
  PORT,
  NATIVE,
  POINTER,

  OBJECT=POINTER,

  // user object types
  SYMBOL,
  LIST,

  // utility object types
  ALIST,
  TABLE,
  BUFFER,

  // interpreter object types
  CHUNK,
  CLOSURE,
  CONTROL
};

#define N_DTYPES (CONTROL+1)

struct object {
  object_t* next;    // invasive linked list of live objects
  uword hash  : 48;  // cached hash code
  uword flags :  8;  // misc flags
  uword type  :  6;  // object type
  uword black :  1;  // GC black flag
  uword gray  :  1;  // GC gray flag
};

#define HEADER object_t obj

// user object types
struct symbol {
  HEADER;
  symbol_t* left, * right;
  char* name;
  uint64 idno;
};

struct list {
  HEADER;
  usize arity;
  value_t head;
  list_t* tail;
};

struct alist {
  HEADER;
  value_t* data;
  usize cnt, cap;
};

struct table {
  HEADER;
  value_t* data;
  usize cnt, cap;
};

enum encoding {
  BYTES,
  ASCII
};

struct buffer {
  HEADER;
  void* data;
  usize cnt, cap;
  int elSize;
  encoding_t encoding;
};

// interpreter object types
struct chunk {
  HEADER;
  alist_t*  vars;
  alist_t*  vals;
  buffer_t* code;
};

struct closure {
  HEADER;
  chunk_t* code;
  alist_t* envt;
};

struct frame {
  chunk_t* fn;
  ushort* ip;
  int bp, fl, pp;
};

struct control {
  HEADER;
  frame_t frame;
  int sp, fp;
  frame_t* frames;
  value_t* values;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// object flags ---------------------------------------------------------------
enum {
  // general flags
  FROZEN   =0x80,
  HASHED   =0x40,
  NODEALLOC=0x20,
  NOFREE   =0x10,

  // environment/ns/chunk flags
  TOPLEVEL =0x08,
  VARIADIC =0x04,

  // frame flags
  CAPTURED =0x01
};

// empty singletons -----------------------------------------------------------
extern list_t EmptyList;

// tags -----------------------------------------------------------------------
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define NUMTAG   0x0000000000000000ul // dummy tag for 64-bit floats
#define ATMTAG   0x7ffc000000000000ul // 32-bit immediate (full type in next 16 bits)
#define IOSTAG   0x7ffd000000000000ul // pointer of type port_t
#define FUNTAG   0x7ffe000000000000ul // pointer of type native_t
#define OBJTAG   0xffff000000000000ul // pointer of type object_t (full type in header)

#define TAGMASK   0xffff000000000000ul
#define WIDEMASK  0xffffffff00000000ul
#define VALMASK   0x0000fffffffffffful
#define SMALLMASK 0x00000000fffffffful

#define NILTAG    (ATMTAG|(((uword)UNIT)  << 32))
#define BOOLTAG   (ATMTAG|(((uword)BOOL)  << 32))
#define GLYPHTAG  (ATMTAG|(((uword)GLYPH) << 32))

#define TRUE      (BOOLTAG | 1)
#define FALSE     (BOOLTAG | 0)
#define NIL       (NILTAG  | 0)
#define NOTFOUND  (NILTAG  | 1)
#define UNDEFINED (NILTAG  | 3)

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// cast/access/test functions -------------------------------------------------
number_t as_number( value_t x );
glyph_t as_glyph( value_t x );
port_t as_port( value_t x );
native_t as_native( value_t x );
pointer_t as_pointer( value_t x ); 
object_t* as_object( value_t x );
symbol_t* as_symbol( value_t x );
list_t* as_list( value_t x );
alist_t* as_alist( value_t x );
table_t* as_table( value_t x );
buffer_t* as_buffer( value_t x );
chunk_t*   as_chunk( value_t x );
closure_t* as_closure( value_t x );
control_t* as_control( value_t x );

bool is_number( value_t x );
bool is_glyph( value_t x );
bool is_port( value_t x );
bool is_native( value_t x );
bool is_unit( value_t x );
bool is_object( value_t x );
bool is_symbol( value_t x );
bool is_list( value_t x );
bool is_alist( value_t x );
bool is_table( value_t x );
bool is_buffer( value_t x );
bool is_chunk( value_t x );
bool is_closure( value_t x );
bool is_control( value_t x );

long intval( value_t x );
uword wrdval( value_t x );

#define hasfl( x, f ) generic((x), value_t:value_hasfl, default: object_hasfl)(x, f)
bool object_hasfl( void* obj, flags fl );
bool value_hasfl( value_t val, flags fl );

#define setfl( x, f ) generic((x), value_t:value_setfl, default: object_setfl)(x, f)
bool object_setfl( void* obj, flags fl );
bool value_setfl( value_t val, flags fl );

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
value_t glyph( glyph_t g );
value_t port( port_t p );
value_t native( native_t n );
value_t pointer( pointer_t p );
value_t object( void* o );

symbol_t* symbol( char* name );
symbol_t* gensym( char* name );
list_t* list( value_t head, list_t* tail );

// canonical constructors -----------------------------------------------------
list_t* mk_list( usize n, value_t* a );
alist_t* mk_alist( usize n, value_t* a );
chunk_t* mk_chunk( alist_t* vars );
closure_t* mk_closure( chunk_t* code, alist_t* envt );
control_t* mk_control( frame_t* f, int sp, int fp, int pp, frame_t* frames, value_t* values );

// list API
value_t list_nth( list_t* l, usize n );

// alist API 
void init_alist( alist_t* slf );
void free_alist( alist_t* slf );
void reset_alist( alist_t* slf );
usize resize_alist( alist_t* slf, usize n );
usize alist_push( alist_t* slf, value_t val );
usize alist_write( alist_t* slf, usize n, value_t* vals );
value_t alist_pop( alist_t* slf );

// table API
void init_table( table_t* slf );
void free_table( table_t* slf );
void reset_table( table_t* slf );
usize resize_table( table_t* slf, usize n );
value_t* table_find( table_t* slf, value_t k );
value_t table_get( table_t* slf, value_t k );
value_t table_add( table_t* slf, value_t k, value_t v );
value_t table_set( table_t* slf, value_t k, value_t v );
value_t table_del( table_t* slf, value_t k );

// buffer API
void init_buffer( buffer_t* slf, int elSize, encoding_t encoding );
void free_buffer( buffer_t* slf );
void reset_buffer( buffer_t* slf );
usize resize_buffer( buffer_t* slf, usize n );
usize buffer_write( buffer_t* slf, usize n, void* data );

// initialization -------------------------------------------------------------
void toplevel_init_object( void );

#endif
