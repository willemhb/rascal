#ifndef object_h
#define object_h

#include "common.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  // not a type
  NOTYPE,

  // object types
  SYMBOL,
  CONS,
  BINARY,
  VECTOR,
  TABLE,
  FUNCTION,

  // value types
  OBJECT=FUNCTION,
  NUMBER,
  GLYPH,
  PORT,
  UNIT
} type_t;

typedef enum {
  // general flags
  BLACK    =0x80000000,
  GRAY     =0x40000000,
  ISALLOC  =0x20000000,
  ISFROZEN =0x10000000,

  // symbol flags
  INTERNED =0x08000000,
  LITERAL  =0x04000000,

  // cons flags
  DOTTED   =0x08000000,

  // binary flags
  ISENCODED=0x08000000,

  // table flags
  IDTABLE  =0x08000000,
  ISMAPPING=0x04000000,

  // function flags
  VARIADIC =0x08000000,
  NOEVAL   =0x04000000,
  BUILTIN  =0x01000000,
  NATIVE   =0x02000000,
  CLOSURE  =0x03000000
} objfl_t;

struct object {
  object_t* next;
  type_t    type;
  flags     flags;
};

#define HEADER object_t obj

// user object types
struct symbol {
  HEADER;
  char* name;
  value_t constant;
  uint64 idno, hash;
  symbol_t* left, * right;
};

struct cons {
  HEADER;
  value_t car, cdr;
};

struct binary {
  HEADER;
  void* data;
  usize cnt, cap;
};

struct vector {
  HEADER;
  value_t* data;
  usize cnt, cap;
};

struct table {
  HEADER;
  cons_t** data;
  usize cnt, dcap, ocap;
  union {
    void*   ord;
    sint8*  ord8;
    sint16* ord16;
    sint32* ord32;
    sint64* ord64;
  };
};

struct function {
  HEADER;
  symbol_t* name;

  union {
    uint16    builtin;
    value_t (*native)(usize n, value_t* a);
    struct {
      cons_t*   nmspc;
      vector_t* envt;
      vector_t* vals;
      binary_t* code;
    };
  };
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// tags -----------------------------------------------------------------------
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define GLYPHTAG 0x7ffc000000000000ul
#define NILTAG   0x7ffd000000000000ul
#define IOSTAG   0x7ffe000000000000ul
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
bool is_cons( value_t x );
bool is_binary( value_t x );
bool is_vector( value_t x );
bool is_table( value_t x );
bool is_function( value_t x );
bool is_object( value_t x );
bool is_number( value_t x );
bool is_glyph( value_t x );
bool is_port( value_t x );
bool is_unit( value_t x );

// miscellaneous predicates ---------------------------------------------------
bool is_constant( value_t x );
bool is_list( value_t x );
bool is_list_of( cons_t* xs, bool (*test)(value_t x) );
bool is_vector_of( vector_t* xs, bool (*test)(value_t x) );

// casts ----------------------------------------------------------------------
symbol_t*   as_symbol( value_t x );
cons_t*     as_cons( value_t x );
binary_t*   as_binary( value_t x );
vector_t*   as_vector( value_t x );
table_t*    as_table( value_t x );
function_t* as_function( value_t x );
object_t*   as_object( value_t x );
number_t    as_number( value_t x );
glyph_t     as_glyph( value_t x );
port_t*     as_port( value_t x );

// constructors ---------------------------------------------------------------
symbol_t*  make_symbol( char* name, bool intern );
cons_t*    make_cons( value_t car, value_t cdr );
binary_t*  make_binary( int elSize, bool encoded, usize n, void* data );
vector_t*  make_vector( usize n, value_t* args );
table_t*   make_table( bool id, usize n, value_t* args );
value_t    make_object( void* ptr );
value_t    make_number( number_t x );
value_t    make_glyph( int x );
value_t    make_port( port_t* ios );

// 

#endif
