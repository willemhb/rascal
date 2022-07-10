#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>
#include "ctypes.h"
#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

typedef struct object_t   object_t;
typedef struct sequence_t sequence_t;

typedef struct cons_t     cons_t;
typedef struct closure_t  closure_t;
typedef struct symbol_t   symbol_t;
typedef struct port_t     port_t;
typedef union  mapping_t  mapping_t;
typedef struct tuple_t    tuple_t;
typedef struct binary_t   binary_t;

typedef struct vector_t   vector_t;

typedef struct string_t   string_t;
typedef struct bytecode_t bytecode_t;


typedef struct node_t node_t;
typedef struct root_t root_t;

// immediate types
typedef long integer_t;
typedef enum builtin_t builtin_t;
typedef enum type_t type_t;

// function pointer types
typedef value_t   (*Cbuiltin_t)( value_t *a, int n );

// tag system - the vector, table, and binary tags are extensible
#define tag_cons      0x00
#define tag_symbol    0x01
#define tag_closure   0x02
#define tag_port      0x03
#define tag_mapping   0x04
#define tag_tuple     0x05
#define tag_binary    0x06
#define tag_immediate 0x07

#define tag_null      (0x00|tag_immediate)

#define tag_none      (0x10|tag_immediate)
#define tag_integer   (0x10|tag_immediate)
#define tag_float     (0x20|tag_immediate)
#define tag_boolean   (0x30|tag_immediate)
#define tag_builtin   (0x40|tag_immediate)
#define tag_character (0x50|tag_immediate)
#define tag_type      (0xf0|tag_immediate)

#define tag_table     (((type_table)<<8)|tag_type)

#define tag_string    (((type_string)<<8)|tag_type)
#define tag_bytecode  (((type_bytecode)<<8)|tag_type)

enum type_t {
  type_cons      = tag_cons,
  type_symbol    = tag_symbol,
  type_closure   = tag_closure,
  type_port      = tag_port,

  type_none      = tag_none,
  type_null      = tag_null,
  
  type_table     = 0x10 | tag_mapping,

  type_vector    = 0x10 | tag_tuple,

  type_string    = 0x10 | tag_binary,
  type_bytecode  = 0x20 | tag_binary,

  type_integer   = tag_integer,
  type_character = tag_character,
  type_boolean   = tag_boolean,
  type_builtin   = tag_builtin,
  type_type      = tag_type
};

struct closure_t {
  value_t name;
  value_t envt;
  value_t values;
  value_t code;
};

struct cons_t {
  value_t car, cdr;
};

struct object_t    object_type(void*);
struct sequence_t  collection_type(void*);

struct symbol_t {
  struct object_type(char) base;
  value_t bind;
  hash_t  hash;
};

struct node_t {
  struct object_type(char) base;
  value_t bind;
  hash_t   hash;
  node_t *left, *right;
};

struct root_t     collection_type(node_t);
struct tuple_t    collection_type(value_t);
struct binary_t   collection_type(uchar);
struct string_t   collection_type(char);
struct bytecode_t collection_type(ushort);
struct vector_t   collection_type(value_t);

union mapping_t {
  root_t root;
  node_t node;
};

struct port_t {
  struct collection_type(char) base; // for the buffer
  int line, col, pos, token;
  FILE     *ios;
  char     name[1];
};

#define TYPE_PAD 256

// type dispatch --------------------------------------------------------------
extern char    *TypeNames[TYPE_PAD];
extern size_t   TypeSizes[TYPE_PAD];
extern size_t   TypeElSize[TYPE_PAD];

#endif
