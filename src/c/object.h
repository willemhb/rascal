#ifndef object_h
#define object_h

#include "common.h"

#include "util/number.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct object {
  object_t* next; // invasive linked list of live objects
  type_t* type;
  uint64 hash  : 48; // 
  uint64 flags : 14;
  uint64 black :  1;
  uint64 gray  :  1;
};

#define HEADER object_t obj

// user object types
struct symbol {
  HEADER;
  value_t left, right;
  char*   name;
  uint64  idno;
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
  value_t* data;
  usize cnt, cap;
};

struct record {
  HEADER;
  value_t slots;
};

struct function {
  HEADER;
  value_t name;
  value_t template;
};

struct method_table {
  HEADER;
  value_t cache;
  value_t thunk;
  value_t varMethods;
  value_t fixMethods;
};

struct method {
  HEADER;
  value_t signature;
  value_t handler;
};

enum datatype {
  NOTYPE, // not a datatype
  NUMBER=1,
  GLYPH,
  PORT,
  UNIT,
  POINTER,

  SYMBOL,
  CONS,
  BINARY,
  VECTOR,
  TABLE,
  RECORD,

  FUNCTION,
  METHOD_TABLE,
  METHOD,

  TYPE,
  CHUNK,
  CONTROL
};

#define NDTYPES (CONTROL+1)

enum kind {
  BOTTOM_TYPE=1,
  UNIT_TYPE,
  DATA_TYPE,
  UNION_TYPE,
  TOP_TYPE
};

struct type {
  HEADER;
  value_t left, right; // union types are invasive trees
  value_t name;
  value_t ctor;
  value_t slots;
  uint64 idno;
  datatype_t datatype;
  kind_t kind;
};

struct chunk {
  HEADER;
  value_t name;
  value_t namespc;
  value_t vals;
  value_t instr;
  value_t envt;
};

struct control {
  HEADER;
  // copy of main registers
  value_t stack;
  int ip, bp, fp, pp;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// object flags ---------------------------------------------------------------
enum {
  // general flags
  NOMANAGE =0x2000, // don't trace
  NODEALLOC=0x1000, // don't free object
  NOFREE   =0x0800, // don't free object data
  FROZEN   =0x0400, // immutable
  HASHED   =0x0200, // cached hash

  // symbol flags
  INTERNED =0x0100,
  LITERAL  =0x0080,

  // cons flags
  DOTTED   =0x0100,

  // binary flags
  ISENCODED=0x0100,

  // table flags
  HASHID   =0x0100,

  // function/method flags
  VARIADIC =0x0100
};

// builtin types --------------------------------------------------------------
extern type_t NumberType, GlyphType, PortType, UnitType, PointerType,
  SymbolType, ConsType, BinaryType, VectorType, TableType, RecordType,
  FunctionType, MethodTableType, MethodType,
  TypeType, ChunkType, ControlType,
  NoneType, AnyType;

// tags -----------------------------------------------------------------------
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define NUMTAG   0x0000000000000000ul // dummy
#define CHRTAG   0x7ffc000000000000ul
#define IOSTAG   0x7ffd000000000000ul
#define NILTAG   0x7ffe000000000000ul
#define PTRTAG   0x7fff000000000000ul
#define FIXTAG   0xfffc000000000000ul
#define OBJTAG   0xffff000000000000ul

#define TAGMASK   0xffff000000000000ul
#define VALMASK   0x0000fffffffffffful

#define NIL       (NILTAG|0)
#define NOTFOUND  (NILTAG|1)
#define UNDEFINED (NILTAG|3)
#define UNBOUND   (NILTAG|5)

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// cast/access/test macros ----------------------------------------------------
#define asnum(x) (((ieee64_t)(x)).dbl)
#define aschr(x) ((glyph_t)((x) & VALMASK))
#define asport(x) ((port_t)((x) & VALMASK))
#define asptr(x) ((pointer_t)((x) & VALMASK))
#define asob(x) ((object_t*)((x) & VALMASK))
#define assym(x) ((symbol_t*)((x) & VALMASK))
#define ascons(x) ((cons_t*)((x) & VALMASK))
#define asbin(x) ((binary_t*)((x) & VALMASK))
#define asvec(x) ((vector_t*)((x) & VALMASK))
#define astb(x) ((table_t*)((x) & VALMASK))
#define asrecord(x) ((record_t*)((x) & VALMASK))
#define asfunc(x) ((function_t*)((x) & VALMASK))
#define asmt(x) ((method_table_t*)((x) & VALMASK))
#define asmethod(x) ((method_t*)((x) & VALMASK))
#define astype(x) ((type_t*)((x) & VALMASK))
#define aschunk(x) ((chunk_t*)((x) & VALMASK))
#define asctl(x) ((control_t*)((x) & VALMASK))

#define isob(x) (((x) & TAGMASK) == OBJTAG)

bool isnum( value_t x );
bool issym( value_t x );
bool iscons( value_t x );

#define intval(x) ((long)asnum(x))
#define wrdval(x) ((uword)asnum(x))

#define tagptr(x) (((value_t)(x)) | PTRTAG)
#define tagob(x) (((value_t)(x)) | OBJTAG)
#define tagchr(x) (((value_t)(x)) | CHRTAG)
#define tagnum(x) (((ieee64_t)((number_t)(x))).word)

#define hasfl(x, fl) (!!(asob(x)->flags & (fl)))
#define setfl(x, fl) (asob(x)->flags |= (fl))
#define sethash(x, h)                            \
  do {                                           \
    value_t _x = x;                              \
    obhash(_x) = h;                              \
    obfl(_x) |= HASHED;                          \
  } while (false)

type_t* type_of( value_t x );
usize size_of_val( value_t x );
usize size_of_dtype( datatype_t dt );

#define size_of(x) generic((x), value_t:size_of_val, datatype_t:size_of_dtype)(x)

void mark_value( value_t slf );
void free_value( value_t slf );

// high level constructors
value_t number( number_t num );
value_t glyph( glyph_t ch );
value_t symbol( char* name );
value_t gensym( char* name );
value_t cons( value_t car, value_t cdr );
value_t vector( usize n, ... );

// accessors/mutators/&c
usize binary_write( binary_t* slf, usize n, void* data );
usize vector_push( vector_t* slf, value_t v );
value_t vector_pop( vector_t* slf );
value_t table_get( table_t* slf, value_t k );
value_t table_set( table_t* slf, value_t k, value_t v );
value_t table_put( table_t* slf, value_t k, value_t v );
value_t table_del( table_t* slf, value_t k );


// working with methods
value_t make_signature( usize n, ... );
void add_method( value_t func, value_t sig, value_t handler, flags fl );

// initialization
void toplevel_init_object( void );

#endif
