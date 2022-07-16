#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>

#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

#define tag_fixnum    0x00 // *00
#define tag_immediate 0x01 // *01
#define tag_cons      0x02 // 010
#define tag_symbol    0x03 // 011
#define tag_function  0x06 // 110
#define tag_object    0x07 // 111

#define tag_character (0x0a000000|tag_immediate)

// these live above the valid unicode range
#define val_true      (0x02000000|tag_immediate)
#define val_false     (0x03000000|tag_immediate)
#define val_nil       (0x08000000|tag_immediate)

typedef enum {
  type_fixnum,
  type_cons,
  type_symbol,
  type_nil,
  type_builtin,
  type_vector
} type_t;

typedef struct {
  type_t type;

  ushort offset;        // additional words allocated for the header (before object)
  uchar  flags;         // discretionary
  uchar  encoding :  3;
  uchar  boxed    :  1;
  uchar  Ctype    :  3;
  uchar  inlined  :  1;
  uchar space[0];
} object_t;

typedef struct {
  size_t length, size; // size gives the true size (in allocation units). length is abstract
  object_t base;
} big_object_t;

typedef struct {
  big_object_t base;
  value_t data[0];
} vector_t;

typedef struct {
  ulong idno, hash;
  value_t bind, flags;
  char name[1];
} symbol_t;

#define type_pad 16

typedef long fixnum_t;
typedef void (*builtin_t)( size_t n );

typedef struct {
  value_t car;
  value_t cdr;
} cons_t;

typedef struct symbols_t {
  struct symbols_t *left, *right;
  symbol_t base;
} symbols_t;

typedef enum {
  /* 0-argument codes */
  op_noop, op_done,

  /* 1-argument opcodes */
  op_loadc
} opcode_t;


#define uval(x)       ((x)>>2)
#define ival(x)       ((long)uval(x))
#define pval(x)       ((void*)(((value_t)(x))&~7ul))

#define gettag(x)     (((value_t)(x))&7)
#define settag(x, t)  ((((value_t)(x))&~7ul)|(t))
#define cpytag(p, x)  settag(p, gettag(x))

#endif
