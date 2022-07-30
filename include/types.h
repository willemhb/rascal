#ifndef rascal_types_h
#define rascal_types_h

#include <stdio.h>

#include "strutils.h"
#include "numutils.h"

#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

/* value tags */
#define tag_object     0b00000000
#define tag_header     0b00000001
#define tag_fixnum     0b00000010
#define tag_immediate  0b00000011

/* header flags */
#define flag_gray      0b00000100
#define flag_traversed 0b00001000

/* fixnum flags (only meaningful when the number is used as part of an object representation) */
#define flag_inlined   0b00000100
#define flag_shared    0b00001000

/* immediate tags */
#define tag_boolean    0b00000011
#define tag_character  0b00000111
#define tag_type       0b00001011
#define tag_singleton  0b00001111 // used to represent empty collection types and other sentinels

/* header tags (indicate object repr) */
// core reprs
#define tag_symbol       0b11110001
#define tag_pair         0b11100001
#define tag_function     0b11010001

// internal reprs
#define tag_bytecode     0b11000001
#define tag_port         0b10110001

// collection reprs
#define tag_tuple        0b10100001
#define tag_vector       0b10010001
#define tag_table        0b10000001

// low-level reprs
#define tag_primitive    0b01110001
#define tag_bitvector    0b01100001
#define tag_bitarray     0b01010001
#define tag_bitstruct    0b01000001

typedef struct {
  value_t type;
  value_t bind;
  idno_t  idno;
  hash_t  hash;
  flags_t flags;
  uint    length;
  char    name[0];
} symbol_t;

typedef struct {
  value_t type;
  value_t car;
  value_t cdr;

  union {
    size_t  length; // for cons
    hash_t  hash;   // for assc
  };
} pair_t;

typedef struct {
  value_t type;
  value_t name;
  value_t envt;

  union {
    value_t (*builtin)( index_t base, size_t argc );
    value_t template;
  };
} function_t;

typedef struct {
  value_t type;
  short  *data;
  value_t length;
  value_t values[0];
} bytecode_t;

typedef struct {
  value_t type;
  uchar  *buffer;
  value_t length;

  FILE   *ios;
  int bufpos, flags, tok, ptok, line, col;
  value_t val, pval;
  char name[0];
} port_t;

typedef struct {
  value_t  type;
  value_t *data;
  value_t  length;

  union {
    value_t parent;  // 
    value_t spec;    // 
  };

  value_t space[0];
} tuple_t;

typedef struct {
  value_t  type;
  value_t *data;
  value_t  length;

  union {
    value_t cache;
    value_t parent;
  };

  uint bitmap, depth;

  value_t space[0];
} vector_t;

typedef struct {
  value_t  type;
  value_t *data;
  value_t  length;

  union {
    value_t parent;
    value_t cache;
  };

  uint bitmap, depth;

  value_t space[0];
} table_t;

typedef struct {
  value_t type;

  union {
    void     *pval;
    uint64_t  u64val;
    int64_t   s64val;
    double    f64val;
    float     f32val;
  };
} primitive_t;

typedef struct {
  value_t type;
  uchar  *data;
  value_t length;

  union {
    value_t Ctype;
    value_t parent;
  };

  uchar space[0];
} bitvector_t;

typedef struct {
  value_t type;
  uchar  *data;
  value_t length;

  union {
    value_t parent;
    value_t Ctype;
  };

  value_t dimensions;

  uchar space[0];
} bitarray_t;

typedef struct {
  value_t type;
  uchar  *data;
  value_t length;

  union {
    value_t parent;
    value_t spec;
  };

  uchar space[0];
} bitstruct_t;

#endif
