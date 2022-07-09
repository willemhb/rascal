#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>
#include "ctypes.h"
#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

typedef struct symbol_t   symbol_t;
typedef struct cons_t     cons_t;
typedef struct tuple_t    tuple_t;
typedef union  mapping_t  mapping_t;
typedef struct binary_t   binary_t;
typedef struct port_t     port_t;
typedef struct closure_t  closure_t;

// table members
typedef struct node_t     node_t;
typedef struct root_t     root_t;

// immediate types
typedef ulong    fixnum_t;
typedef uint     integer_t;
typedef float    float_t;
typedef bool     boolean_t;
typedef char     character_t;

typedef union instruction_t instruction_t;
typedef enum  builtin_t     builtin_t;
typedef enum  form_t        form_t;
typedef enum  opcode_t      opcode_t;

// aliases
typedef tuple_t   vector_t;
typedef binary_t  string_t;
typedef binary_t  bytecode_t;
typedef mapping_t table_t;
typedef mapping_t dict_t;

// function pointer types
typedef value_t   (*Cbuiltin_t)( value_t *a, int n );
typedef value_t   (*Cform_t)( value_t *a, int n, value_t *e );
typedef void      (*ensure_t)( value_t *a, int n );

// tag system - the vector, table, and binary tags are extensible
#define tag_cons      0x00
#define tag_symbol    0x01
#define tag_port      0x02
#define tag_closure   0x03
#define tag_tuple     0x04
#define tag_vector    0x04
#define tag_mapping   0x05
#define tag_table     0x05
#define tag_binary    0x06
#define tag_immediate 0x07

typedef enum {
   type_cons      = tag_cons,
   type_symbol    = tag_symbol,
   type_vector    = tag_tuple,
   type_table     = tag_mapping,
   type_string    = tag_binary,
   type_port      = tag_port,
   type_closure   = tag_closure,

   // weird types
   type_none      = 0x00 | tag_immediate,
   type_any       = 0x10 | tag_immediate,
   type_null      = 0x20 | tag_immediate,
   type_type      = 0x30 | tag_immediate,

   // numeric types
   type_fixnum    = 0x40 | tag_immediate,
   type_integer   = 0x50 | tag_immediate,
   type_float     = 0x60 | tag_immediate,

   // other primitives
   type_character = 0x70 | tag_immediate,
   type_boolean   = 0x80 | tag_immediate,
   type_byte      = 0x90 | tag_immediate,

   // numeric representations of builtin functions
   type_builtin   = 0xa0 | tag_immediate,
   type_form      = 0xb0 | tag_immediate,
   type_opcode    = 0xc0 | tag_immediate,

   // table types extended (on object)
   type_dict      = 0x10 | tag_mapping,

   type_bytecode  = 0x20 | tag_binary,
} type_t;

#define TYPE_PAD 256

struct cons_t {
  value_t car, cdr;
};

struct node_t {
  value_t bind;

  union {
    void    *ptr;
    char    *key;
    value_t  val;
  };

  hash_t   hash;

  node_t *left, *right;
};

struct root_t   collection_type(node_t);
struct tuple_t  collection_type(value_t);
struct binary_t collection_type(uchar);

union mapping_t {
  root_t root;
  node_t node;
};

struct symbol_t {
  value_t idno, bind;
  hash_t  hash;
  char   name[1];
};

struct port_t {
  value_t  value, flags;
  
  uint line, col, pos, token;
  FILE     *ios;
  binary_t *buffer;
  char     name[1];
};

struct closure_t {
  value_t name;
  value_t envt;
  value_t values;
  value_t code;
};

// type dispatch --------------------------------------------------------------
char    *TypeNames[TYPE_PAD];
size_t   TypeSizes[TYPE_PAD];
Ctype_t  TypeCtypes[TYPE_PAD];
Ctype_t  TypeEltypes[TYPE_PAD]; // array only

int     (*Sizeof[TYPE_PAD])( value_t x );
value_t (*Relocate[TYPE_PAD])( value_t x );
int     (*Print[TYPE_PAD])( FILE *ios, value_t x );
int     (*Order[TYPE_PAD])( value_t x, value_t y );

#endif
