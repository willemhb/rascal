#ifndef value_h
#define value_h

#include "common.h"

// C types --------------------------------------------------------------------
// value types ----------------------------------------------------------------
typedef uintptr_t value_t; // tagged lisp data
typedef uintptr_t fixnum_t;
typedef uint32 small_t;
typedef uint32 glyph_t;
typedef double real_t;
typedef bool boolean_t;
typedef void *pointer_t;
typedef struct object_t object_t;
typedef struct type_t type_t;

// small types ----------------------------------------------------------------
typedef uint8 uint8_t;
typedef sint8 sint8_t;
typedef uint16 uint16_t;
typedef sint16 sint16_t;
typedef uint32 uint32_t;
typedef sint32 sint32_t;
typedef float  real32_t;

// glyph types ----------------------------------------------------------------
typedef char ascii_t;
typedef uint8 latin1_t;
typedef uint8 utf8_t;
typedef uint16 utf16_t;
typedef uint32 utf32_t;

// object types ---------------------------------------------------------------
// metaobject types -----------------------------------------------------------
typedef struct data_type_t  data_type_t;
typedef struct union_type_t union_type_t;

// internal types -------------------------------------------------------------
typedef struct arr_node_t   arr_node_t;
typedef struct arr_leaf_t   arr_leaf_t;
typedef struct map_node_t   map_node_t;
typedef struct map_leaf_t   map_leaf_t;
typedef struct map_leaves_t map_leaves_t;
typedef struct set_node_t   set_node_t;
typedef struct set_leaf_t   set_leaf_t;
typedef struct set_leaves_t set_leaves_t;
typedef struct method_table_t method_table_t;
typedef struct method_t method_t;
typedef struct chunk_t chunk_t;
typedef struct closure_t closure_t;
typedef struct variable_t variable_t;
typedef struct namespace_t namespace_t;
typedef struct environment_t environment_t;
typedef struct control_t control_t;

// user types -----------------------------------------------------------------
typedef struct symbol_t symbol_t;
typedef struct function_t function_t;
typedef struct port_t port_t;
typedef struct binary_t binary_t;
typedef struct string_t string_t;
typedef struct tuple_t tuple_t;
typedef struct list_t list_t;
typedef struct vector_t vector_t;
typedef struct dict_t dict_t;
typedef struct set_t set_t;
typedef struct table_t table_t;
typedef struct alist_t alist_t;
typedef struct buffer_t buffer_t;
typedef struct struct_t struct_t;
typedef struct record_t record_t;
typedef struct complex_t complex_t;
typedef struct ratio_t ratio_t;
typedef struct big_t big_t;

// builtin type codes ---------------------------------------------------------
typedef enum {
  // metaobject types ---------------------------------------------------------
  DATA_TYPE,
  UNION_TYPE,

  // internal types -----------------------------------------------------------
  ARR_NODE,
  ARR_LEAF,
  MAP_NODE,
  MAP_LEAF,
  MAP_LEAVES,
  SET_NODE,
  SET_LEAF,
  SET_LEAVES,
  METHOD_TABLE,
  METHOD,
  CHUNK,
  CLOSURE,
  VARIABLE,
  NAMESPACE,
  ENVIRONMENT,
  CONTROL,

  // user types ---------------------------------------------------------------
  SYMBOL,
  FUNCTION,
  PORT,
  BINARY,
  STRING,
  TUPLE,
  LIST,
  VECTOR,
  DICT,
  SET,
  TABLE,
  ALIST,
  BUFFER,
  STRUCT,
  RECORD,
  COMPLEX,
  RATIO,
  BIG
} obj_type_t;

typedef enum {
  // numeric ------------------------------------------------------------------
  OBJECT=BIG,
  SINT8,
  UINT8,
  SINT16,
  UINT16,
  SINT32,
  UINT32,
  REAL32,
  FIXNUM,
  REAL,

  // glyph --------------------------------------------------------------------
  ASCII,
  LATIN1,
  UTF8,
  UTF16,
  UTF32,

  // misc ---------------------------------------------------------------------
  BOOLEAN,
  UNIT,

  // internal -----------------------------------------------------------------
  POINTER,

  // fucked up types ----------------------------------------------------------
  NONE,
  ANY
} val_type_t;

typedef enum {
  BOTTOM_KIND,
  DATA_KIND,
  UNION_KIND,
  TOP_KIND
} kind_t;

struct object_t {
  object_t *next;
  uword hash   : 48;
  uword flags  :  6;
  uword hashed :  1;
  uword frozen :  1;
  uword type   :  6;
  uword gray   :  1;
  uword black  :  1;
};

#define HEADER object_t obj

// globals --------------------------------------------------------------------
// tags -----------------------------------------------------------------------
#define QNAN        0x7ff8000000000000ul

// user values ----------------------------------------------------------------
#define SMALLTAG    0x7ffc000000000000ul
#define OBJTAG      0x7ffd000000000000ul
#define FIXTAG      0x7ffe000000000000ul
#define PTRTAG      0x7fff000000000000ul

// common small tags ----------------------------------------------------------
#define BOOLTAG     (SMALLTAG | (((uword)BOOLEAN) << 32))
#define NULTAG      (SMALLTAG | (((uword)UNIT) << 32))

#define TAG_MASK    0xffff000000000000ul
#define VAL_MASK    0x0000fffffffffffful

#define TRUE_VAL    (BOOLTAG|1ul)
#define FALSE_VAL   (BOOLTAG|0ul)

#define NUL         (NULTAG|0ul)
#define UNBOUND     (NULTAG|1ul)
#define UNDEFINED   (NULTAG|3ul)
#define NOTFOUND    (NULTAG|5ul)

#define FIXNUM_MAX  VAL_MASK
#define FULL_MASK  (TAG_MASK|VAL_MASK)

#define NUM_TYPES (ANY+1)

extern type_t* BuiltinTypes[NUM_TYPES];

// API ------------------------------------------------------------------------
// tags, tagging, types, queries ----------------------------------------------
#define ASA(x, t) ((t*)(((uword)(x)) & VAL_MASK))

val_type_t val_type(value_t val);

obj_type_t val_obj_type(value_t val);
obj_type_t obj_obj_type(void* ptr);
#define obj_type(x) generic2(obj_type, x)

data_type_t* val_type_of(value_t val);
data_type_t* obj_type_of(void* ptr);
#define type_of(x) generic2(type_of, x)

// lifetimes ------------------------------------------------------------------
void mark_value(value_t val);

// core APIs ------------------------------------------------------------------
kind_t val_has_type(value_t val, type_t* type);
kind_t obj_has_type(void* ptr, type_t* type);

#define has_type(x, t) generic2(has_type, x, t)

void val_print(value_t, port_t* ios);
void obj_print(void* ptr, port_t* ios);

#define rl_print(x, p) generic2(print, x, p)

usize val_size_of(value_t val);
usize obj_size_of(void* ptr);

#define rl_size_of(x) generic2(size_of, x)

uhash val_hash(value_t x);
uhash obj_hash(void* ptr);

#define rl_hash(x) generic2(hash, x)

bool val_equal(value_t x, value_t y);
bool obj_equal(void* px, void* py);

#define rl_equal(x, y) generic2(equal, x, y)

int val_compare(value_t x, value_t y);
int obj_compare(void* x, void* y);

#define rl_compare(x, y) generic2(compare, x, y)

bool has_flag(void* ptr, flags fl);
bool set_flag(void* ptr, flags fl);
bool clear_flag(void* ptr, flags fl);

#endif
