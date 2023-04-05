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
  function_t* next;      // next method to try
  type_t*     type;      // constructed type (if any)
  list_t*     signature; // argument signature
  value_t     template;  // thing to be called
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

struct variable {
  HEADER;
  variable_t* next;
  long        index;
};

struct chunk {
  HEADER;
  list_t*   ns;
  alist_t*  vals;
  buffer_t* code;
};

struct closure {
  HEADER;
  chunk_t* chunk;
  alist_t* envt;
};

struct control {
  HEADER;
  control_t* caller;
  closure_t* callee;
  uint16*    ip;
  alist_t*   stack;
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

// APIs


#endif
