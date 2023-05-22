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
  BLACK   =0x8000,
  GRAY    =0x4000,
  NOFREE  =0x2000,
  FROZEN  =0x1000,

  // symbol flags
  INTERNED=0x0001,
  LITERAL =0x0002,
  CONSTANT=0x0004,

  // table flags
  IDTABLE =0x0001,
  EQTABLE =0x0002,
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
  value_t bind; // toplevel
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
extern struct {
  usize counter;
  symbol_t* root;
} SymbolTable;

// empty singletons -----------------------------------------------------------
extern list_t EmptyList;

// tags
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define GLYPHTAG 0x7ffc000000000000ul
#define NILTAG   0x7ffd000000000000ul
#define PTRTAG   0x7ffe000000000000ul
#define OBJTAG   0xffff000000000000ul

#define TAGMASK 0xffff000000000000ul
#define VALMASK 0x0000fffffffffffful

#define NIL       (NILTAG|0)
#define NOTFOUND  (NILTAG|1)
#define UNDEFINED (NILTAG|3)
#define UNBOUND   (NILTAG|5)

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// queries --------------------------------------------------------------------


// type predicates ------------------------------------------------------------

#endif
