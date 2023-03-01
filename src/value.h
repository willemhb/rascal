#ifndef value_h
#define value_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef uintptr_t  value_t;
typedef double     real_t;
typedef uintptr_t  fixnum_t;
typedef bool       bool_t;
typedef value_t  (*native_t)(usize n, value_t* args);

typedef struct object_t  object_t;
typedef struct symbol_t  symbol_t;
typedef struct tuple_t   tuple_t;
typedef struct list_t    list_t;
typedef struct vector_t  vector_t;
typedef struct binary_t  binary_t;
typedef struct stencil_t stencil_t;

typedef enum {
  NONE,
  UNIT,
  BOOL,
  NATIVE,
  REAL,
  FIXNUM,
  SYMBOL,
  TUPLE,
  LIST,
  VECTOR,
  BINARY,
  STENCIL,  // internal bitmapped vector type
  ANY
} type_t;

typedef enum {
  HASHED=0x100
} objfl_t;

struct object_t {
  object_t *next;
  uword     hash  : 48;
  uword     flags :  9;
  uword     gray  :  1;
  uword     black :  1;
  uword     type  :  5;
};

#define HEADER object_t obj

struct symbol_t {
  HEADER;
  symbol_t *left, *right;
  char*     name;
  value_t   bind;
  uword     idno;
};

struct tuple_t {
  HEADER;
  usize   len;
  value_t slots[];
};

struct list_t {
  HEADER;
  usize   len;
  value_t head;
  list_t* tail;
};

struct vector_t {
  HEADER;
  usize      len;
  stencil_t* vals;
};

struct binary_t {
  HEADER;
  usize  len;
  ubyte  array[];
};

struct stencil_t {
  HEADER;
  usize bitmap;
  value_t array[];
};

// globals --------------------------------------------------------------------
#define QNAN        0x7ff8000000000000ul
#define FIXNUMTAG   0x7ffc000000000000ul
#define INTTAG      0x7ffd000000000000ul
#define NUL         0x7ffe000000000000ul
#define BOOLTAG     0x7fff000000000000ul
#define GLYPHTAG    0xfffc000000000000ul
#define SYSPTRTAG   0xfffd000000000000ul
#define NATIVETAG   0xfffe000000000000ul
#define OBJTAG      0xffff000000000000ul

#define TAG_MASK    0xffff000000000000ul
#define VAL_MASK    0x0000fffffffffffful

#define TRUE_VAL    (BOOLTAG|1ul)
#define FALSE_VAL   (BOOLTAG|0ul)

#define UNBOUND     (NUL|1ul)
#define NOTFOUND    (NUL|3ul)

#define tag_of(x)   ((x)&TAG_MASK)
#define val_of(x)   ((x)&VAL_MASK)

#define NUM_TYPES (ANY+1)

extern tuple_t   EmptyTuple;
extern list_t    EmptyList;
extern vector_t  EmptyVector;
extern binary_t  EmptyBinary;
extern stencil_t EmptyStencil;

// API ------------------------------------------------------------------------
// tags, tagging, types -------------------------------------------------------
type_t  type_of(value_t val);
usize   size_of_val(value_t val);
usize   size_of_obj(void* ptr);
usize   size_of_type(type_t type);
char*   type_name_of(value_t val);
char*   type_name_of_type(type_t type);
bool    has_type(value_t val, type_t type);

#define size_of(x)                              \
  generic((x),                                  \
          type_t:size_of_type,                  \
          value_t:size_of_val,                  \
          object_t*:size_of_obj,                \
          symbol_t*:size_of_obj,                \
          tuple_t*:size_of_obj,                 \
          list_t*:size_of_obj,                  \
          vector_t*:size_of_obj,                \
          binary_t*:size_of_obj,                \
          stencil_t*:size_of_obj)(x)

#define type_name(x)                            \
  generic((x),                                  \
          int: type_name_of_type,               \
          type_t:type_name_of_type,             \
          value_t:type_name_of)(x)

bool    is_object(value_t val);
bool    is_byte(value_t val);
bool    is_function(value_t val);
bool    is_number(value_t val);

value_t tag_ptr(void* p, uword t);
value_t tag_word(uword w, uword t);
value_t tag_dbl(double dbl);

uword   as_word(value_t val);
real_t  as_number(value_t val);
real_t  as_real(value_t val);
void*   as_ptr(value_t val);

bool    has_flag(void* ptr, flags fl);
bool    set_flag(void* ptr, flags fl);
bool    del_flag(void* ptr, flags fl);

#define is_unit(x)     has_type(x, UNIT)
#define is_bool(x)     has_type(x, BOOL)
#define is_native(x)   has_type(x, NATIVE)
#define is_real(x)     has_type(x, REAL)
#define is_fixnum(x)   has_type(x, FIXNUM)
#define is_symbol(x)   has_type(x, SYMBOL)
#define is_tuple(x)    has_type(x, TUPLE)
#define is_list(x)     has_type(x, LIST)
#define is_vector(x)   has_type(x, VECTOR)
#define is_binary(x)   has_type(x, BINARY)
#define is_stencil(x)  has_type(x, STENCIL)

#define as_bool(x)    ((x)==TRUE_VAL)
#define as_fixnum(x)  ((fixnum_t)as_word(x))
#define as_native(x)  ((native_t)as_ptr(x))
#define as_object(x)  ((object_t*)as_ptr(x))
#define as_symbol(x)  ((symbol_t*)as_ptr(x))
#define as_tuple(x)   ((tuple_t*)as_ptr(x))
#define as_list(x)    ((list_t*)as_ptr(x))
#define as_vector(x)  ((vector_t*)as_ptr(x))
#define as_binary(x)  ((binary_t*)as_ptr(x))
#define as_stencil(x) ((stencil_t*)as_ptr(x))

// constructors ---------------------------------------------------------------
value_t symbol(char* name);
value_t tuple(usize n, value_t* args);
value_t cons(value_t head, list_t* tail);
value_t list(usize n, value_t* args);
value_t vector(usize n, value_t* args);
value_t binary(usize n, value_t* args);
value_t stencil(usize n, value_t* args);

// accessors ------------------------------------------------------------------
value_t nth_hd(list_t* xs, usize n);
list_t* nth_tl(list_t* xs, usize n);

value_t vector_ref(vector_t* xs, usize n);
vector_t* vector_set(vector_t* xs, usize n, value_t val);
vector_t* vector_del(vector_t* xs, usize n);
vector_t* vector_add(vector_t* xs, value_t val);
vector_t* vector_rmv(vector_t* xs);

usize stencil_len(stencil_t* xs);
bool stencil_has(stencil_t* xs, usize i);
value_t stencil_nth(stencil_t* xs, usize n);
value_t stencil_ref(stencil_t* xs, usize i);
stencil_t* stencil_update(stencil_t* xs, usize rmv, usize add, value_t* args);

#endif
