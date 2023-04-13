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

struct list {
  HEADER;
  value_t head;
  list_t* tail;
  usize   arity;
};

struct chunk {
  HEADER;
  namespace_t* ns;    // local namespace
  alist_t*     vals;  // compiled constants
  buffer_t*    code;  // instructions
};

typedef enum {
  ABSTRACTION = 0x01,
  SCRIPT      = 0x02
} chunkfl_t;

struct lambda {
  HEADER;
  symbol_t* name;
  uint64    arity;
  chunk_t*  chunk;
};

struct native {
  HEADER;
  symbol_t* name;
  uint64    arity;
  value_t (*callback)(usize n, value_t* args);
};

struct primitive {
  HEADER;
  symbol_t* name;
  uint64    arity;
  uint16    label; // corresponding opcode
};

struct closure {
  HEADER;
  lambda_t*      lambda;
  environment_t* envt;
};

struct namespace {
  HEADER;
  namespace_t* parent;
  table_t*     locals;
  table_t*     upvalues;
};

typedef enum {
  LOCALNS    = 0x01,
  SCRIPTNS   = 0x02,
  TOPLEVELNS = 0x03
} nmspcfl_t;

struct environment {
  HEADER;
  namespace_t* ns;
  alist_t*     binds;
};

typedef enum {
  LOCALENV    = 0x01,
  SCRIPTENV   = 0x02,
  TOPLEVELENV = 0x03
} envfl_t;

struct variable {
  HEADER;
  symbol_t*    name;  // the unqualified name of the variable
  namespace_t* ns;    // namespace where the variable is defined
  table_t*     meta;  // other variable metadata
  value_t      bind;  // could be the binding itself, or the index of the binding (for lexical variables)
};

struct upvalue {
  HEADER;
  variable_t* var;
  value_t*    location;
  value_t     value;
  upvalue_t*  next;
};

struct table {
  HEADER;
  usize     cnt, cap;
  value_t*  entries;
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
#define     as_symbol(x) ((symbol_t*)((x) & WVMASK))
#define     is_symbol(x) rl_isa(x, &SymbolType)

symbol_t*   symbol(char* name, bool intern);
variable_t* resolve(symbol_t* name, namespace_t* ns);
variable_t* define(symbol_t* name, namespace_t* ns);

// list
list_t*     list(usize n, value_t* args);
list_t*     cons(value_t hd, list_t* tl);

// table
#define     as_table(x) ((table_t*)((x) & WVMASK))
#define     is_table(x) rl_isa(x, &TableType)

table_t*    table(void);
void        reset_table(table_t* slf);
void        init_table(table_t* slf);
bool        resize_table(table_t* slf, usize n);
long        table_find(table_t* slf, value_t key);
value_t     table_get(table_t* slf, value_t key);
value_t     table_set(table_t* slf, value_t key, value_t val);
bool        table_add(table_t* slf, value_t key, value_t val);

// alist
alist_t*    alist(void);
void        reset_alist(alist_t* slf);
usize       alist_push(alist_t* slf, value_t val);
value_t     alist_pop(alist_t* slf);

// buffer
buffer_t*   buffer(int elsize, bool encoded);
void        init_buffer(buffer_t* slf, int elsize, int encoded);
void        reset_buffer(buffer_t* slf);
usize       buffer_write(buffer_t* slf, usize cnt, void* dat);

#endif
