#ifndef object_h
#define object_h

#include <stdio.h>

#include "value.h"

// C types
struct object {
  object_t* next;       // invasive live objects list
  uint64    hash  : 48;
  uint64    flags :  8;
  uint64    type  :  6;
  uint64    black :  1;
  uint64    gray  :  1;
  ubyte     space[0];
};

typedef enum {
  FROZEN = 0x80,
  HASHED = 0x40
} objfl_t;

#define HEADER object_t obj

struct symbol {
  HEADER;
  symbol_t* left, * right;
  uint64 idno;
  char*  name;
};

typedef enum {
  INTERNED = 0x01
} symfl_t;

struct type {
  HEADER;
  type_t* left, * right; // for union types
  uint64 idno;
  char*  name;

  // layout info
  usize   size;

  // sacred methods
  void   (*print)(value_t x, port_t* ios);
  uint64 (*hash)(value_t x);
  bool   (*equal)(value_t x, value_t y);
  int    (*compare)(value_t x, value_t y);

  // lifetime methods
  void   (*trace)(void* slf);
  void   (*free)(void* slf);

  // misc methods
  usize  (*size_of)(void* slf);
};

struct function {
  HEADER;
  symbol_t* name;    // function's common name
  type_t*   type;    // type constructed by the function (if any)
  value_t   handler; // chunk (user function), pointer (native C function), or fixnum (opcode)
};

struct port {
  HEADER;
  FILE*     ios;
};

struct list {
  HEADER;
  value_t head;
  list_t* tail;
  usize   arity;
};

struct string {
  HEADER;
  usize length;
  char  chars[];
};

struct namespace {
  HEADER;
  namespace_t* parent;
  table_t* locals;
  table_t* upvalues;
};

struct variable {
  HEADER;
  symbol_t*    name;  // the unqualified name of the variable
  namespace_t* nmspc; // namespace where the variable is defined
  type_t*      type;  // type constraint on the variable
  table_t*     meta;  // other variable metadata
  value_t      bind;  // could be the binding itself, or the index of the binding (for lexical variables)
};

struct upvalue {
  HEADER;
  value_t*   location;
  value_t    value;
  upvalue_t* next;
};

struct chunk {
  HEADER;
  namespace_t* nmspc; // local namespace
  alist_t*     vals;  // compiled constants
  buffer_t*    code;  // instructions
};

struct closure {
  HEADER;
  usize      cnt, cap;
  upvalue_t* upvalues;
  chunk_t*   chunk;
};

struct table {
  HEADER;
  usize     cnt, cap;
  value_t*  table;

  union {
    sint8*  o8;
    sint16* o16;
    sint32* o32;
    sint64* o64;
  };
};

struct alist {
  HEADER;
  usize    cnt, cap;
  value_t* array;
};

struct buffer {
  HEADER;
  usize cnt, cap;
  void* bits;
  int   elsize;
  bool  encoded;
};

// globals
extern type_t SymbolType, TypeType, FunctionType, PortType, ListType,
  StringType, NameSpaceType, VariableType, UpValueType, ChunkType,
  ClosureType, TableType, AlistType, BufferType;

// APIs
// general object api
#define head(obj) ((object_t*)(obj))

#define INIT_HEADER(obj, datatype, fl)            \
  do {                                            \
    extern object_t* LiveObjects;                 \
    object_t* __obj  = (object_t*)(obj);          \
    type_t*   __type = datatype;                  \
                                                  \
    __obj->next  = LiveObjects;                   \
    LiveObjects  = __obj;                         \
    __obj->hash  = 0;                             \
    __obj->flags = fl;                            \
    __obj->type  = __type->idno;                  \
    __obj->gray  = true;                          \
    __obj->black = false;                         \
  } while (false)

// symbol
symbol_t*   symbol(char* name, bool intern);
variable_t* resolve(symbol_t* name, namespace_t* ns);
variable_t* define(symbol_t* name, namespace_t* ns);

// function
function_t* function(symbol_t* name, type_t* type, value_t handler);

// port
// rascal constructor
port_t*     port(FILE* ios);

// interface to C io libraries
port_t*     rlopen(const char* fname, const char* mode);
int         rlclose(port_t* px);
int         rlgetc(port_t* px);
int         rlputc(port_t* px, int ch);
int         rlungetc(port_t* px, int ch);
int         rlpeekc(port_t* px);
int         rlputs(port_t* px, char* s);
string_t*   rlgets(port_t* px, int n);
int         rlprintf(port_t* px, const char* fmt, ...);
int         rlvprintf(port_t* px, const char* fmt, va_list va);

// list
list_t*     list(usize n, value_t* args);
list_t*     cons(value_t hd, list_t* tl);

// string
string_t*   string(char* chars);

// table
table_t*    table(void);
void        reset_table(table_t* slf);
long        table_find(table_t* slf, value_t key);
value_t     table_get(table_t* slf, value_t key);
value_t     table_set(table_t* slf, value_t key, value_t val);
bool        table_add(table_t* slf, value_t key, value_t val);
bool        table_del(table_t* slf, value_t key);

// alist
alist_t*    alist(void);
void        reset_alist(alist_t* slf);
usize       alist_push(alist_t* slf, value_t val);
value_t     alist_pop(alist_t* slf);

// buffer
buffer_t*   buffer(int elsize, bool encoded);
void        buffer_reset(buffer_t* slf);
usize       buffer_write(buffer_t* slf, usize cnt, void* dat);

#endif
