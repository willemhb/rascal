#ifndef object_h
#define object_h

#include "common.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  NOTYPE,
  SYMBOL,
  LIST,
  BINARY,
  VECTOR,
  TABLE,
  NATIVE,
  CLOSURE,
  CHUNK,
  OBJECT=CHUNK,
  NUMBER,
  GLYPH,
  POINTER,
  UNIT
} type_t;

typedef enum {
  // general flags
  BLACK   =0x80000000,
  GRAY    =0x40000000,
  NOFREE  =0x20000000,
  FROZEN  =0x10000000,

  // symbol flags
  INTERNED=0x00010000,
  LITERAL =0x00020000,

  // table flags
  IDTABLE =0x00010000,
  EQTABLE =0x00020000,

  // native/closure flags
  VARIADIC=0x00010000
} objfl_t;

struct object {
  object_t* next;   // presently unused
  type_t    type;
  flags     flags;
};

#define HEADER object_t obj

struct symbol {
  HEADER;
  char* name;
  value_t constant; // toplevel constant binding
  uint64 idno, hash;
  symbol_t* left, * right;
};

struct list {
  HEADER;
  value_t head;
  list_t* tail;
  usize   arity;
};

struct binary {
  HEADER;
  usize cnt, cap;
  void* data;
  int elSize;
  bool encoded;
};

struct vector {
  HEADER;
  usize cnt, cap;
  value_t* data;
};

struct table {
  HEADER;
  usize cnt, cap;
  value_t* data;
  union {
    void*   ord;
    sint8*  ord8;
    sint16* ord16;
    sint32* ord32;
    sint64* ord64;
  };
};

struct native {
  HEADER;
  symbol_t* name;
  value_t (*native)(usize n, value_t* args); // native
};

struct closure {
  HEADER;
  chunk_t*  code;
  vector_t* envt;
};

struct chunk {
  HEADER;
  symbol_t* name;
  list_t*   envt;
  vector_t* vals;
  binary_t* code;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// symbol table ---------------------------------------------------------------
struct SymbolTable {
  usize counter;
  symbol_t* root;
};

// empty singletons -----------------------------------------------------------
extern struct SymbolTable SymbolTable;
extern list_t EmptyList;

// tags
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define GLYPHTAG 0x7ffc000000000000ul
#define NILTAG   0x7ffd000000000000ul
#define PTRTAG   0x7ffe000000000000ul
#define OBJTAG   0xffff000000000000ul

#define TAGMASK  0xffff000000000000ul
#define VALMASK  0x0000fffffffffffful

#define NIL       (NILTAG|0)
#define NOTFOUND  (NILTAG|1)
#define UNDEFINED (NILTAG|3)
#define UNBOUND   (NILTAG|5)

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// queries --------------------------------------------------------------------
type_t type_of( value_t x );

usize  size_of_type( type_t t );
usize  size_of_value( value_t x );

#define size_of(x) _Generic((x), value_t: size_of_value, type_t: size_of_type)(x)

bool has_flag( void* ptr, flags fl );

// type predicates ------------------------------------------------------------
bool is_symbol( value_t x );
bool is_list( value_t x );
bool is_binary( value_t x );
bool is_vector( value_t x );
bool is_table( value_t x );
bool is_native( value_t x );
bool is_closure( value_t x );
bool is_chunk( value_t x );
bool is_object( value_t x );
bool is_number( value_t x );
bool is_glyph( value_t x );
bool is_pointer( value_t x );
bool is_unit( value_t x );

// miscellaneous predicates ---------------------------------------------------
bool is_constant( value_t x );
bool is_list_of( value_t x, bool (*test)(value_t x) );

// casts ----------------------------------------------------------------------
symbol_t*  as_symbol( value_t x );
list_t*    as_list( value_t x );
binary_t*  as_binary( value_t x );
vector_t*  as_vector( value_t x );
table_t*   as_table( value_t x );
native_t*  as_native( value_t x );
closure_t* as_closure( value_t x );
chunk_t*   as_chunk( value_t x );
object_t*  as_object( value_t x );
number_t   as_number( value_t x );
glyph_t    as_glyph( value_t x );
pointer_t  as_pointer( value_t x );

// constructors ---------------------------------------------------------------
symbol_t*  symbol( char* name, bool intern );
list_t*    cons( value_t hd, list_t* tl );
list_t*    list( usize n, value_t* args );
binary_t*  binary( int elSize, bool encoded, usize n, void* data );
vector_t*  vector( usize n, value_t* args );
table_t*   table( bool id, usize n, value_t* args );
native_t*  native( symbol_t* name, value_t (*funcptr)( usize n, value_t* args ) );
closure_t* closure( chunk_t* code, vector_t* envt );
chunk_t*   chunk( symbol_t* name, list_t* envt, bool variadic );
value_t    object( void* ptr );
value_t    number( number_t x );
value_t    glyph( int x );
value_t    pointer( pointer_t x );

// getters, setters, queries, &c ----------------------------------------------
usize binary_size( binary_t* slf, bool cap );
void  reset_binary( binary_t* slf );
void* binary_offset( binary_t* slf, usize n );
usize binary_write( binary_t* slf, usize n, void* data );

usize   vector_size( vector_t* slf, bool cap );
usize   reset_vector( vector_t* slf );
usize   vector_write( vector_t* slf, usize n, value_t* data );
usize   vector_push( vector_t* slf, value_t x );
value_t vector_pop( vector_t* slf );

usize   table_size( table_t* slf, bool cap );
usize   reset_table( table_t* slf );
bool    table_has( table_t* slf, value_t k );
long    table_locate( table_t* slf, value_t k );
long    table_add( table_t* slf, value_t k, value_t v );
value_t table_get( table_t* slf, value_t k );
value_t table_set( table_t* slf, value_t k, value_t v );
value_t table_put( table_t* slf, value_t k, value_t v );
value_t table_del( table_t* slf, value_t k );

#endif
